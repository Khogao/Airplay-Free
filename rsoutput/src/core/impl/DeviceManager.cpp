/* Copyright (c) 2020  Eric Milles <eric.milles@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Debugger.h"
#include "DeviceDiscovery.h"
#include "DeviceManager.h"
#include "ServiceDiscovery.h"
#include "Options.h"
#include "Uncopyable.h"
#include "raop/RAOPDevice.h"
#include "raop/RAOPEngine.h"
#include <cassert>
#include <stdexcept>
#include <string>
#include <iostream>
#include <Poco/Format.h>
#include <Poco/Event.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Timespan.h>
#include <Poco/Timestamp.h>
#include <Poco/Net/StreamSocket.h>


using Poco::Timespan;
using Poco::Timestamp;
using Poco::Net::StreamSocket;


namespace {

class DeviceConnector : public ServiceDiscovery::ResolveListener, public ServiceDiscovery::QueryListener {
public:
    DeviceConnector(const DeviceInfo& device) : _device(device), _port(0), _resolved(false), _sdRef(NULL) {}

    void connect(Poco::Net::StreamSocket& socket) {
        if (_device.isZeroConf()) {
            _sdRef = ServiceDiscovery::resolveService(_device.addr().first, _device.addr().second, *this);
            ServiceDiscovery::start(_sdRef);
            
            try {
                if (!_event.tryWait(5000)) { // 5 seconds timeout
                    if (_sdRef) {
                        ServiceDiscovery::stop(_sdRef);
                        _sdRef = NULL;
                    }
                    throw std::runtime_error("Timeout resolving device service");
                }
            } catch (...) {
                if (_sdRef) {
                    ServiceDiscovery::stop(_sdRef);
                    _sdRef = NULL;
                }
                throw;
            }
            
            if (_sdRef) {
                ServiceDiscovery::stop(_sdRef);
                _sdRef = NULL;
            }

            if (!_resolved) {
                 throw std::runtime_error("Failed to resolve device address");
            }
            socket.connect(_resolvedAddress);
        } else {
            std::string hostAndPort = _device.addr().first + ":" + _device.addr().second;
            socket.connect(Poco::Net::SocketAddress(hostAndPort));
        }
    }

    // ResolveListener
    void onServiceResolved(DNSServiceRef sdRef, std::string fullName, std::string host, uint16_t port, const ServiceDiscovery::TXTRecord& txt) override {
         ServiceDiscovery::stop(sdRef);
         _sdRef = NULL; 
         _port = port;
         _sdRef = ServiceDiscovery::queryService(host, kDNSServiceType_A, *this);
         ServiceDiscovery::start(_sdRef);
    }

    // QueryListener
    void onServiceQueried(DNSServiceRef sdRef, std::string rrname, uint16_t rrtype, uint16_t rdlen, const void* rdata, uint32_t ttl) override {
         ServiceDiscovery::stop(sdRef);
         _sdRef = NULL;
         _resolvedAddress = Poco::Net::SocketAddress(Poco::Net::IPAddress(rdata, rdlen), _port);
         _resolved = true;
         _event.set();
    }

private:
    const DeviceInfo& _device;
    Poco::Event _event;
    DNSServiceRef _sdRef;
    uint16_t _port;
    Poco::Net::SocketAddress _resolvedAddress;
    bool _resolved;
};

}


// shorthand for accessing the implementation type of device output sink
#define RAOP_ENGINE (*(*this).outputSinkForDevices().cast<RAOPEngine>())


DeviceManager::DeviceManager(Player& player, OutputObserver& outputObserver)
:
	_volume(FLT_MIN),
	_player(player),
	_outputObserver(outputObserver),
	_deviceObserver(*this, &DeviceManager::onDeviceChanged)
{
	Options::addObserver(_deviceObserver);
	DeviceDiscovery::browseDevices(*this);
}


DeviceManager::~DeviceManager()
{
	DeviceDiscovery::stopBrowsing(*this);
	Options::removeObserver(_deviceObserver);
}


void DeviceManager::openDevices()
{
	bool anyDeviceActivated = false;

	// hold reference to active options
	const Options::SharedPtr options = Options::getOptions();

	for (DeviceInfoSet::const_iterator it = options->devices().begin();
		it != options->devices().end(); ++it)
	{
		const DeviceInfo& deviceInfo = *it;

		if (options->isActivated(deviceInfo.name()))
		{
			anyDeviceActivated = true;

			openDevice(deviceInfo);
		}
	}

	DeviceInfoSet discovered;
	{
		ScopedLock lock(_mutex);
		discovered = _discoveredDevices;
	}

	for (DeviceInfoSet::const_iterator it = discovered.begin();
		it != discovered.end(); ++it)
	{
		if (options->isActivated(it->name()))
		{
			anyDeviceActivated = true;
			openDevice(*it);
		}
	}

	if (!anyDeviceActivated)
	{
		// limit rate of warning messages because some players will try to open
		// the next track in a playlist if the current track failed and this can
		// cause a long sequence of dialog boxes that must be dismissed manually
		static Timestamp lastAlertTime = 0;
		if (lastAlertTime.elapsed() > (5 * Timespan::SECONDS))
		{
			lastAlertTime.update();

			std::cerr << "No remote speakers are selected for output." << std::endl;
		}
	}
}


void DeviceManager::closeDevices()
{
	ScopedLock lock(_mutex);

	for (DeviceMap::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		DeviceMap::mapped_type device = it->second;

		device->close();
	}
}


bool DeviceManager::isAnyDeviceOpen(const bool ping) const
{
	ScopedLock lock(_mutex);

	for (DeviceMap::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		const DeviceMap::mapped_type device = it->second;

		if (device->isOpen(ping))
		{
			return true;
		}
	}

	return false;
}


void DeviceManager::setVolume(const float level)
{
	ScopedLock lock(_mutex);

	const float delta = volumeSet() ? (level - _volume) : 0;  _volume = level;

	for (DeviceMap::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		DeviceMap::mapped_type device = it->second;

		if (device->isOpen())
		{
			device->setVolume(level, delta);
		}
	}
}


void DeviceManager::setOffset(const time_t offset)
{
	ScopedLock lock(_mutex);

	const time_t length = _outputMetadata.length();
	if (length > 0) { assert( offset <= length ); }

	// calculate timestamps of chapter/track begin and end
	_outputInterval = RAOP_ENGINE.getOutputInterval(length, offset);

	for (DeviceMap::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		DeviceMap::mapped_type device = it->second;

		if (device->isOpen())
		{
			device->updateProgress(_outputInterval);
		}
	}
}


void DeviceManager::setMetadata(const OutputMetadata& metadata)
{
	if (metadata.length() == _outputMetadata.length()
		&& metadata.playlistPos() == _outputMetadata.playlistPos()) return;

	ScopedLock lock(_mutex);

	_outputMetadata = metadata;

	for (DeviceMap::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		DeviceMap::mapped_type device = it->second;

		if (device->isOpen())
		{
			device->updateMetadata(_outputMetadata);
		}
	}
}


void DeviceManager::clearMetadata()
{
	ScopedLock lock(_mutex);

	_outputInterval = OutputInterval();
	_outputMetadata = OutputMetadata();
}


const OutputFormat& DeviceManager::outputFormat() const
{
	return RAOPEngine::outputFormat();
}


OutputSink::SharedPtr DeviceManager::outputSinkForDevices()
{
	if (_deviceOutputSink.isNull())
	{
		_deviceOutputSink = new RAOPEngine(_outputObserver);
	}
	return _deviceOutputSink;
}


Device::SharedPtr DeviceManager::lookupDevice(const uint32_t remoteControlId) const
{
	if (remoteControlId > 0)
	{
		ScopedLock lock(_mutex);

		for (DeviceMap::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
		{
			const DeviceMap::mapped_type device = it->second;

			if (device->remoteControlId() == remoteControlId)
			{
				return device;
			}
		}
	}

	return NULL;
}


//------------------------------------------------------------------------------


void DeviceManager::onDeviceFound(const DeviceInfo& device)
{
	{
		ScopedLock lock(_mutex);
		_discoveredDevices.insert(device);
	}

	if (_deviceOutputSink.referenceCount() > 1)
	{
		openDevice(device);
	}
}


void DeviceManager::onDeviceLost(const DeviceInfo& device)
{
	{
		ScopedLock lock(_mutex);
		_discoveredDevices.erase(device);
	}

	destroyDevice(device);
}


Device::SharedPtr DeviceManager::createDevice(const DeviceInfo& deviceInfo)
{
	const std::string& auth = deviceInfo.isZeroConf() ? DeviceDiscovery::getDeviceAuth(deviceInfo) : "";

	switch (LOWORD(deviceInfo.type()))
	{
	case DeviceInfo::APX:
		return new RAOPDevice(RAOP_ENGINE, auth,
			RAOPDevice::ET_NONE, RAOPDevice::MD_NONE);

	case DeviceInfo::AS3:
		return new RAOPDevice(RAOP_ENGINE, auth,
			RAOPDevice::ET_SECURED, RAOPDevice::MD_NONE);

	case DeviceInfo::AS4:
	case DeviceInfo::ATV:
	case DeviceInfo::AVR:
		return new RAOPDevice(RAOP_ENGINE, auth,
			RAOPDevice::ET_NONE, RAOPDevice::MD_TEXT
							   | RAOPDevice::MD_IMAGE
							   | RAOPDevice::MD_PROGRESS);

	case DeviceInfo::AFS:
		return new RAOPDevice(RAOP_ENGINE, auth,
			RAOPDevice::ET_SECURED, RAOPDevice::MD_TEXT
								  | RAOPDevice::MD_IMAGE
								  | RAOPDevice::MD_PROGRESS);

	case DeviceInfo::ANY:
		return new RAOPDevice(RAOP_ENGINE, auth,
			// check device type bit-field for encryption and metadata settings
			!!(HIWORD(deviceInfo.type()) & 0x08), (HIWORD(deviceInfo.type()) & 0x07));

	default:
		const std::string message(Poco::format(
			"Unable to playback to remote speakers \"%s\".\n"
			"Device type (%i) is unsupported at this time.",
			deviceInfo.name(), (int) deviceInfo.type()));
		std::cerr << message << std::endl;

		throw std::runtime_error(message);
	}
}


void DeviceManager::destroyDevice(const DeviceInfo& deviceInfo)
{
	ScopedLock lock(_mutex);

	_devices.erase(deviceInfo.name());
}


void DeviceManager::openDevice(const DeviceInfo& deviceInfo)
{
	try
	{
		ScopedLock lock(_mutex);

		if (_devices.count(deviceInfo.name()) == 0)
		{
			Device::SharedPtr device = createDevice(deviceInfo);

			_devices[deviceInfo.name()] = device;

			// run dialog box that will asynchronously resolve service name to
			// host and port, resolve host to IP address and connect to address
			// and port
            StreamSocket socket;
            try {
                DeviceConnector connector(deviceInfo);
                connector.connect(socket);
            } catch (const std::exception& e) {
				const std::string message(Poco::format(
					"Unable to connect to remote speakers \"%s\": %s",
					deviceInfo.name(), std::string(e.what())));
				throw std::runtime_error(message);
            }

			Debugger::printf("Connected to remote speakers \"%s\" at %s.",
				deviceInfo.name().c_str(), socket.peerAddress().toString().c_str());

			int returnCode = device->test(socket, true);

			// check if remote speakers require a password
			while (returnCode == 401)
			{
				Options::SharedPtr options = Options::getOptions();

				// check for password in options
				if (options->getPassword(deviceInfo.name()).empty())
				{
					// prompt user for password
                    std::cerr << "Password required for " << deviceInfo.name() << " but dialog not supported." << std::endl;
					throw std::invalid_argument("Password required but not provided.");
				}

				device->setPassword(options->getPassword(deviceInfo.name()));

				returnCode = device->test(socket, false);

				// check if password was not accepted
				if (returnCode == 401)
				{
					options->clearPassword(deviceInfo.name());
				}

				// repeat until password is accepted or user cancels
			}

			device->close();

			// check for initiation error
			if (returnCode)
			{
				const std::string message(Poco::format(
					"Unable to initiate session with remote speakers \"%s\".\n"
					"Error code: %i", deviceInfo.name(), returnCode));
				std::cerr << message << std::endl;

				throw std::runtime_error(message);
			}
		}

		DeviceMap::mapped_type device = _devices[deviceInfo.name()];

		if (!device->isOpen())
		{
			if (!isAnyDeviceOpen())
			{
				// before opening the first device, init shared session state
				RAOP_ENGINE.reinit(_outputInterval);
			}

			// run dialog box that will asynchronously resolve service name to
			// host and port, resolve host to IP address and connect to address
			// and port
            StreamSocket socket;
            try {
                DeviceConnector connector(deviceInfo);
                connector.connect(socket);
            } catch (const std::exception& e) {
				const std::string message(Poco::format(
					"Unable to connect to remote speakers \"%s\": %s",
					deviceInfo.name(), std::string(e.what())));
				throw std::runtime_error(message);
            }

			AudioJackStatus audioJackStatus = AUDIO_JACK_CONNECTED;

			// negotiate session parameters with remote speakers
			int returnCode = device->open(socket, audioJackStatus);

			// check if remote speakers require a password
			while (returnCode == 401)
			{
				Options::SharedPtr options = Options::getOptions();

				// check for password in options
				if (options->getPassword(deviceInfo.name()).empty())
				{
					// prompt user for password
                    std::cerr << "Password required for " << deviceInfo.name() << " but dialog not supported." << std::endl;
					throw std::invalid_argument("Password required but not provided.");
				}

				device->setPassword(options->getPassword(deviceInfo.name()));

				// negotiate session parameters with remote speakers again
				returnCode = device->open(socket, audioJackStatus);

				// check if password was not accepted
				if (returnCode == 401)
				{
					options->clearPassword(deviceInfo.name());
				}

				// repeat until password is accepted or user cancels
			}

			// check for negotiation error
			if (returnCode)
			{
				if (returnCode == 453)
				{
					const std::string message(Poco::format(
						"Remote speakers \"%s\" are in use by another player.",
						deviceInfo.name()));
					std::cerr << message << std::endl;

					throw std::runtime_error(message);
				}
				else
				{
					const std::string message(Poco::format(
						"Unable to connect to remote speakers \"%s\".\n"
						"Error code: %i", deviceInfo.name(), returnCode));
					std::cerr << message << std::endl;

					throw std::runtime_error(message);
				}
			}
			else if (audioJackStatus == AUDIO_JACK_DISCONNECTED
				&& LOWORD(deviceInfo.type()) != DeviceInfo::AVR)
			{
				const std::string message(Poco::format(
					"Audio jack on remote speakers \"%s\" is not connected.",
					deviceInfo.name()));
				std::cerr << message << std::endl;
			}

			if (deviceInfo.type() == DeviceInfo::AVR) device->getVolume();
			if (volumeSet()) device->setVolume(_volume, 0);

			if (_outputMetadata.length() > 0 || !_outputMetadata.title().empty())
			{
				device->updateMetadata(_outputMetadata);
				device->updateProgress(_outputInterval);
			}
		}
		else if (_outputMetadata.length() > 0)
		{
			device->updateProgress(_outputInterval);
		}

		return; // bypass exception handling
	}
	CATCH_ALL

	Options::getOptions()->setActivated(deviceInfo.name(), false);
	Options::postNotification(
		new DeviceNotification(DeviceNotification::DEACTIVATE, deviceInfo));
}


void DeviceManager::onDeviceChanged(DeviceNotification* const notification)
{
	try
	{
		switch (notification->changeType())
		{
		case DeviceNotification::ACTIVATE:
			// open device if open for playback to pick it up immediately
			if (_deviceOutputSink.referenceCount() > 1)
			{
				openDevice(notification->deviceInfo());
			}
			break;

		case DeviceNotification::DEACTIVATE:
			destroyDevice(notification->deviceInfo());
			break;
		}
	}
	CATCH_ALL
}
