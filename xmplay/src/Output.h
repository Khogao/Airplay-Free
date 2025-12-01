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

#ifndef Output_h
#define Output_h


#include <memory>
#include <xmpout.h>


typedef std::auto_ptr<class CriticalSection> CriticalSectionPtr;
typedef std::auto_ptr<class OutputComponent> OutputComponentPtr;


class Output
{
public:
	static INT_PTR CALLBACK optionsDialogProc(HWND, UINT, WPARAM, LPARAM);

	static const char* WINAPI getName(DWORD);
	static DWORD WINAPI getFlags(DWORD);

	static BOOL WINAPI open(DWORD, XMPOUT_FORMAT*, HANDLE);
	static void WINAPI close();
	static BOOL WINAPI reset();

	static BOOL WINAPI write(const void*, DWORD);

	static DWORD WINAPI canWrite();
	static DWORD WINAPI getBuffered();

	static BOOL WINAPI setPaused(BOOL);
	static void WINAPI setVolumeAndBalance(float, float);

	static void WINAPI onNewTrack(const char*);

private:
	static void onBytesOutput(size_t);

	static OutputComponentPtr _output;
	static CriticalSectionPtr _mutex;
	static HANDLE             _event;
	static BOOL               _reset;

	Output();
};


#endif // Output_h
