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

#include "XMPlayPlayer.h"
#include "Platform.inl"
#include "Debugger.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>


XMPlayPlayer::XMPlayPlayer(XMPFUNC_MISC* miscFunc, XMPFUNC_STATUS* statusFunc)
:
	_miscFunc(miscFunc),
	_statusFunc(statusFunc)
{
}


BYTE XMPlayPlayer::majorVersion() const
{
	DWORD playerVersion = _miscFunc->GetVersion();
	BYTE majorVersion = HIBYTE(HIWORD(playerVersion));
	return majorVersion;
}


BYTE XMPlayPlayer::minorVersion() const
{
	DWORD playerVersion = _miscFunc->GetVersion();
	BYTE minorVersion = LOBYTE(HIWORD(playerVersion));
	return minorVersion;
}


HWND XMPlayPlayer::window() const
{
	return _miscFunc->GetWindow();
}


void XMPlayPlayer::play()
{
	_miscFunc->PerformShortcut(80); // play/pause
}


void XMPlayPlayer::pause()
{
	_miscFunc->PerformShortcut(80); // play/pause
}


DWORD WINAPI stopImpl(LPVOID context)
{
	return ((XMPFUNC_MISC*) context)->PerformShortcut(81); // stop
}

void XMPlayPlayer::stop()
{
	// stop shortcut is executed in calling thread and triggers Output::close;
	// perform shortcut asynchronously in worker thread to avoid deadlock of
	// thread waiting for itself to exit
	QueueUserWorkItem(stopImpl, _miscFunc, WT_EXECUTEDEFAULT);
}


DWORD WINAPI restartImpl(LPVOID context)
{
	return ((XMPFUNC_MISC*) context)->PerformShortcut(84); // restart track
}

void XMPlayPlayer::restart()
{
	// restart shortcut is executed in calling thread and triggers Output::close;
	// perform shortcut asynchronously in worker thread to avoid deadlock of
	// thread waiting for itself to exit
	QueueUserWorkItem(restartImpl, _miscFunc, WT_EXECUTEDEFAULT);
}


void XMPlayPlayer::startNext()
{
	_miscFunc->PerformShortcut(128); // next track
}


void XMPlayPlayer::startPrev()
{
	_miscFunc->PerformShortcut(129); // previous track
}


void XMPlayPlayer::increaseVolume()
{
	_miscFunc->PerformShortcut(512); // volume up
	_miscFunc->PerformShortcut(512); // each call produces a 2% change in volume
}


void XMPlayPlayer::decreaseVolume()
{
	_miscFunc->PerformShortcut(513); // volume down
	_miscFunc->PerformShortcut(513); // each call produces a 2% change in volume
}


void XMPlayPlayer::toggleMute()
{
	_miscFunc->PerformShortcut(523); // toggle volume mute
}


void XMPlayPlayer::toggleShuffle()
{
	_miscFunc->PerformShortcut(313); // toggle random play order
}


time_t XMPlayPlayer::getPlaybackMetadata(shorts_t& listpos,
	std::string& title, std::string& album, std::string& artist) const
{
	time_t length = 0; listpos = shorts_t(0,0);
	title.clear(), album.clear(), artist.clear();

	// query player for the current playlist's length and cursor position
	static const unsigned short_max = std::numeric_limits<short>::max();
	const unsigned len = sendIpcMessage(IPC_GETLISTLENGTH);
	if (len > 0 && len <= short_max)
	{
		const unsigned pos = sendIpcMessage(IPC_GETLISTPOS);
		if (pos <= short_max)
		{
			assert(pos < len);
			listpos = std::make_pair(pos + 1, len);
		}
	}

	// create a smart pointer for holding the player-managed strings
	std::tr1::shared_ptr<char> xmpString(
		_miscFunc->GetInfoText(XMPINFO_TEXT_GENERAL), _miscFunc->Free);
	const std::string infoString(xmpString ? xmpString.get() : "");

	std::string::size_type beg = 0;
	while (beg != std::string::npos)
	{
		std::string::size_type mid = infoString.find_first_of('\t', beg);
		std::string::size_type end = infoString.find_first_of('\n', beg);

		if (mid != std::string::npos && mid < end)
		{
			const std::string key(infoString.substr(beg, mid - beg));
			++mid; // advance past '\t'
			const std::string val(infoString.substr(mid, end - mid));

			if (key == "Length")
			{
				unsigned int mins = 0, secs = 0;
				sscanf_s(val.c_str(), "%u:%u", &mins, &secs);
				// calculate length in milliseconds
				length = (mins * 60000) + (secs * 1000);
			}
			else if (key == "Title")
			{
				title = val;
			}
			else if (key == "File" && (title.empty() || title == val))
			{
				char file[MAX_PATH];  title = (0 == _splitpath_s(val.c_str(),
					NULL, 0, NULL, 0, file, sizeof(file), NULL, 0) ? file : val);
			}
		}

		beg = infoString.find_first_not_of('\n', end);
	}

	xmpString.reset(_miscFunc->GetTag(TAG_ALBUM), _miscFunc->Free);
	if (xmpString) album = xmpString.get();

	xmpString.reset(_miscFunc->GetTag(TAG_ARTIST), _miscFunc->Free);
	if (xmpString) artist = xmpString.get();

	return std::max<time_t>(length, 0);
}


time_t XMPlayPlayer::getPlaybackPosition() const
{
	const double offset = _statusFunc->GetTime();

	if (offset >= 0.001 && offset < (std::numeric_limits<time_t>::max() / 1000))
	{
		// convert from seconds to milliseconds
		return static_cast<time_t>(offset * 1000.0);
	}

	return 0;
}


DWORD_PTR XMPlayPlayer::sendIpcMessage(LPARAM lParam, WPARAM wParam) const
{
	assert(lParam > 0);
	assert(window() > 0);
	DWORD_PTR result = 0;

	if (!SendMessageTimeout(window(), WM_XMP_IPC, wParam, lParam, SMTO_NORMAL, 250, &result))
	{
		DWORD error = GetLastError();

		if (error == ERROR_TIMEOUT)
		{
			Debugger::printf("%s(%li, %i) timed out", __FUNCTION__, lParam, wParam);
		}
		else
		{
			Debugger::printf("%s(%li, %i) failed with error: %s", __FUNCTION__, lParam, wParam, Platform::Error::describe(error).c_str());
		}

		return 0;
	}

	return result;
}
