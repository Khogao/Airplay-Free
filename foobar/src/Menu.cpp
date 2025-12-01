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

#pragma region mainmenu_item_with_popup

// simplify creation of single main menu item (command) with attached submenu (group popup + commands)

struct mainmenu_item_and_group_popup_glue : public mainmenu_group_popup_impl
{
	class mainmenu_item_with_popup* const m_submenu;

	mainmenu_item_and_group_popup_glue(
		const GUID& p_guid,
		const GUID& p_parent,
		t_uint32    p_priority,
		const char* const p_name,
		class mainmenu_item_with_popup* const p_submenu)
	:
		mainmenu_group_popup_impl(p_guid, p_parent, p_priority, p_name), m_submenu(p_submenu)
	{
	}

	t_uint32 get_sort_priority(); // overridden to provide timely call to m_submenu's display_init()
};


class mainmenu_item_with_popup : private service_factory_single_t<mainmenu_item_and_group_popup_glue>, public mainmenu_commands
{
	friend mainmenu_item_and_group_popup_glue; // so it can call display_init()

public:
	mainmenu_item_with_popup(
		const char* const name,
		const char* const guid,
		const GUID& parent_guid,
		const t_uint32 sort_priority = sort_priority_dontcare)
	:
		service_factory_single_t<mainmenu_item_and_group_popup_glue>(
			pfc::GUID_from_text(guid), parent_guid, sort_priority, name, this)
	{
	}

private:
	virtual void display_init() {}; // called just before menu item is displayed

	bool get_display(t_uint32 index, pfc::string_base& string, t_uint32& flags) {
		mainmenu_commands::get_display(index, string, flags);
		get_flags(index, flags);
		return true;
	}

	virtual void get_flags(t_uint32 index, t_uint32& flags) {}

	GUID get_parent() { return get_static_instance().get_guid(); }
};


t_uint32 mainmenu_item_and_group_popup_glue::get_sort_priority()
{
	m_submenu->display_init();
	return mainmenu_group_popup_impl::get_sort_priority();
}


template <class T>
class mainmenu_item_with_popup_factory_t : public mainmenu_commands_factory_t<T> {};

#pragma endregion

//------------------------------------------------------------------------------

class Menu : public mainmenu_item_with_popup
{
public:
	Menu();

	void display_init();
	t_uint32 get_command_count();

	GUID get_command    (t_uint32 index);
	void get_flags      (t_uint32 index, t_uint32& flags);
	void get_name       (t_uint32 index, pfc::string_base& string);
	bool get_description(t_uint32 index, pfc::string_base& string);
	void execute        (t_uint32 index,    ::service_ptr_t<service_base> unused);

private:
	DeviceUtils deviceUtils;
	DeviceUtils::InfoListPtr devices;
	const DeviceUtils::Info& device_at(const t_uint32 i) const { return (*devices)[i]; }
};

static const mainmenu_item_with_popup_factory_t<Menu> mainmenu_item_with_popup_factory_g;

//------------------------------------------------------------------------------

Menu::Menu()
:
	// add menu item to the playback menu; it should appear just under the 'Devices' menu item (GUID is one higher)
	mainmenu_item_with_popup("Remote Devices", "{5ECFCDC7-60CD-47D7-B544-531E60AB2550}", mainmenu_groups::playback_etc)
{
}


void Menu::display_init()
{
	devices = deviceUtils.enumerate();
}


t_uint32 Menu::get_command_count()
{
	return (devices ? devices->size() : 0);
}


GUID Menu::get_command(const t_uint32 index)
{
	if (index >= get_command_count()) uBugCheck();

	static_api_ptr_t<hasher_md5> md5;
	std::string string(device_at(index).display_name);
	const GUID guid(md5->process_single_guid(string.c_str(), string.length()));

	return guid;
}


void Menu::get_flags(const t_uint32 index, t_uint32& flags)
{
	if (index >= get_command_count()) uBugCheck();

	flags |= flag_defaulthidden;
	if (device_at(index).activated) flags |= flag_checked;
	if (device_at(index).available) flags |= flag_disabled;
}


void Menu::get_name(const t_uint32 index, pfc::string_base& string)
{
	if (index >= get_command_count()) uBugCheck();

	string += device_at(index).display_name.c_str();
}


bool Menu::get_description(const t_uint32 index, pfc::string_base& string)
{
	if (index >= get_command_count()) uBugCheck();

	string += (!device_at(index).activated ? "Activates" : "Deactivates");
	string += " '";
	string += device_at(index).display_name.c_str();
	string += "' in the ";
	string += Plugin::name().c_str();
	string += " DSP.";

	return true;
}


void Menu::execute(const t_uint32 index, service_ptr_t<service_base> reserved)
{
	if (index >= get_command_count()) uBugCheck();

	deviceUtils.toggle(device_at(index).display_name);
}
