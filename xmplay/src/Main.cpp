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
#include "OptionsUtils.h"
#include "Output.h"
#include "Plugin.h"
#include "XMPlayPlayer.h"
#include <cassert>
#include <memory>
#include <string>
#include <shlobj.h>


HINSTANCE dllInstance = NULL;
std::auto_ptr<XMPlayPlayer> player;
static std::string iniFilePath;


/**
 * Main entry point for dynamic-link library (DLL).  This function is called
 * when the DLL is attached to or detached from a process or thread.
 *
 * @param instance
 * @param reason
 * @param reserved
 * @return <code>TRUE</code> on success, <code>FALSE</code> on failure
 */
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		Debugger::print("Player attaching to plug-in...");
		assert(dllInstance == NULL);
		dllInstance = instance;
		break;

	case DLL_PROCESS_DETACH:
		Debugger::print("Player detaching from plug-in...");
		OptionsUtils::saveOptions(iniFilePath);
		assert(dllInstance != NULL);
		dllInstance = NULL;
		break;
	}

	return TRUE;
}


/**
 * Main entry point for XMPlay output plug-in.  This function is called when the
 * player registers the plug-in.
 *
 * @return address of output structure to be used as interface between player
 *         and plug-in
 */
extern "C" __declspec(dllexport) XMPOUT* XMPOUT_GetInterface(DWORD face,
	InterfaceProc faceproc)
{
	if (face == 0)
	{
		Debugger::print("Player requesting plug-in interface...");

		// initialize output interface singleton
		static XMPOUT out = {
			0,         // flags
			"Devices", // label
			Output::optionsDialogProc,
			Output::getName,
			Output::getFlags,
			Output::open,
			Output::close,
			Output::reset,
			Output::setPaused,
			Output::setVolumeAndBalance,
			Output::canWrite,
			Output::write,
			Output::getBuffered,
			NULL, // getGeneralInfo function
			Output::onNewTrack
		};

		if (player.get() == NULL)
		{
			// initialize player interface singleton
			player.reset(new XMPlayPlayer(
				(XMPFUNC_MISC*) faceproc(XMPFUNC_MISC_FACE),
				(XMPFUNC_STATUS*) faceproc(XMPFUNC_STATUS_FACE)));

			// initialize plug-in options file path
			CHAR pathBuffer[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathBuffer)))
			{
				iniFilePath.assign(pathBuffer);
				iniFilePath.append("\\");
				iniFilePath.append(Plugin::name());
				SHCreateDirectoryExA(NULL, iniFilePath.c_str(), NULL);
				iniFilePath.append("\\plugin.ini");
			}
			else
			{
				DWORD pathLength = GetModuleFileNameA(dllInstance, pathBuffer, MAX_PATH);
				if (pathLength > 0 && pathLength <= MAX_PATH)
				{
					const std::string pathString(pathBuffer, pathLength);
					const std::string::size_type pos = pathString.find_last_of('\\');
					if (pos != std::string::npos)
						iniFilePath.assign(pathString.substr(0, pos + 1));
				}
				iniFilePath.append("xmplay.ini");
			}

			// load plug-in options from file
			OptionsUtils::loadOptions(iniFilePath);
		}

		return &out;
	}
	else
	{
		return NULL;
	}
}
