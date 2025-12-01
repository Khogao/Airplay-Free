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
#include "Resources.h"

namespace Options
{
	namespace
	{
		INT_PTR CALLBACK optionsDialogProc(HWND dialogWindow, UINT message, WPARAM wParam, LPARAM lParam)
		{
			static std::auto_ptr<OptionsDialog> optionsDialog;

			switch (message)
			{
			case WM_INITDIALOG:
				CenterWindowOverParent(dialogWindow);

				optionsDialog.reset(new OptionsDialog);
				// set callback that will keep apply button updated
				optionsDialog->setStatusCallback(std::tr1::bind(
					SetWindowEnabled, GetDlgItem(dialogWindow, IDOK), std::tr1::placeholders::_1));
				// create plug-in options pane within dialog window
				optionsDialog->doModeless(GetDlgItem(dialogWindow, DIALOG_OPTIONS_CHILD_FRAME));
				break;

			case WM_DESTROY:
				optionsDialog.reset();
				break;

			case WM_CLOSE:
				EndDialog(dialogWindow, 1);
				break;

			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					switch (LOWORD(wParam))
					{
					case IDOK:
						optionsDialog->doApply();
						break;
					case IDCANCEL:
						EndDialog(dialogWindow, 2);
						break;
					}
				}
				break;
			}

			return 0;
		}
	}


	void show_popup(HWND parentWindow)
	{
		WIN32_OP(DialogBox(core_api::get_my_instance(),
			MAKEINTRESOURCE(DIALOG_OPTIONS), parentWindow, &optionsDialogProc) > 0)
	}
}
