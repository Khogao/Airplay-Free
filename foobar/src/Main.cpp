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

#include "StdAfx.h"

#include <foobar2000/foobar2000_component_client/component_client.cpp>

DECLARE_COMPONENT_VERSION(
	Plugin::name().c_str(),
	Plugin::version().c_str(),
	Plugin::aboutText().c_str());

static std::auto_ptr<Player> player;
std::auto_ptr<OutputComponent> output;

//------------------------------------------------------------------------------

class Main : public initquit
{
public:
	void on_init();
	void on_quit();

private:
	std::string iniFilePath;
};

static const initquit_factory_t<Main> initquit_factory_g;

//------------------------------------------------------------------------------

#include "FoobarPlayer.h"
#include "Platform.inl"
#include <shlobj.h>


void Main::on_init()
{
	try
	{
		// bind to player's console
		Debugger::setPrintCallback(console::print);

		// initialize plug-in options file path
		const char* const profilePath = core_api::get_profile_path();
		if (profilePath != NULL && std::strncmp(profilePath, "file://", 7) == 0)
		{
			std::string path, name;
			Platform::Charset::fromUTF8(profilePath + 7, path);
			Platform::Charset::fromUTF8(core_api::get_my_file_name(), name);
			iniFilePath.assign(path + "\\configuration\\" + name + ".dll.ini");
		}
		else
		{
			CHAR pathBuffer[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathBuffer)))
			{
				iniFilePath.assign(pathBuffer);
				iniFilePath.append("\\").append(Plugin::name());
				SHCreateDirectoryExA(NULL, iniFilePath.c_str(), NULL);
				iniFilePath.append("\\plugin.ini");
			}
			else
			{
				std::string filePath(core_api::get_my_full_path());
				const std::string::size_type pos = filePath.find_last_of('.');
				if (pos != std::string::npos)
				{
					filePath = filePath.substr(0, pos);
				}
				Platform::Charset::fromUTF8(filePath + ".ini", iniFilePath);
			}
		}

		// load plug-in options from file
		OptionsUtils::loadOptions(iniFilePath);

		player.reset(new FoobarPlayer);
		output.reset(new OutputComponent(*player));
	}
	CATCH_ALL
}


void Main::on_quit()
{
	try
	{
		std::auto_ptr<Player         > fp(player);
		std::auto_ptr<OutputComponent> oc(output);

		// save plug-in options to file
		OptionsUtils::saveOptions(iniFilePath);
	}
	CATCH_ALL
}
