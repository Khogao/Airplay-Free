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

#include <foobar2000/ATLHelpers/ATLHelpers.h> // includes all foobar2000 headers

#include <cassert>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <random>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>

#include <set>
#include <map>
#include <list>
#include <array>
#include <queue>
#include <stack>
#include <tuple>
#include <vector>

#include "Player.h"
#include "Plugin.h"
#include "Debugger.h"
#include "DeviceUtils.h"
#include "OptionsUtils.h"
#include "OptionsDialog.h"
#include "OutputComponent.h"

namespace Options { void show_popup(HWND = core_api::get_main_window()); }

//------------------------------------------------------------------------------

namespace core_api
{
	namespace
	{
		typedef std::tr1::function<void (void)> callback_t;

		class main_thread_callback_t : public main_thread_callback
		{
		public:
			main_thread_callback_t(callback_t callback) : _callback(callback) {}
			void callback_run() { _callback(); }
		private:
			callback_t _callback;
		};
	}

	inline void run_in_main_thread(callback_t callback)
	{
		if (is_main_thread())
		{
			callback();
		}
		else
		{
			main_thread_callback_spawn<main_thread_callback_t>(callback);
		}
	}
}
