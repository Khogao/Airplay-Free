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

#include "Output.h"
#include "Debugger.h"
#include "OptionsDialog.h"
#include "OutputComponent.h"
#include "Platform.h"
#include "Plugin.h"
#include "Resources.h"
#include "Uncopyable.h"
#include "XMPlayPlayer.h"
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>


#define ASSERT_NOT_NULL(ptr) if ((ptr).get() == NULL) throw (#ptr " is null")

#define SYNC_BLOCK_WITH(mtx) const ScopedLock $_(mtx)


class CriticalSection : private Uncopyable
{
	CRITICAL_SECTION cs;

public:
	 CriticalSection() throw() { InitializeCriticalSection(&cs); }
	~CriticalSection() throw() {     DeleteCriticalSection(&cs); }

	void enter() throw() { EnterCriticalSection(&cs); }
	void leave() throw() { LeaveCriticalSection(&cs); }
};


class ScopedLock : private Uncopyable
{
	CriticalSection& cs;

public:
	 ScopedLock(CriticalSection& cs) throw() : cs(cs) { cs.enter(); }
	~ScopedLock() throw()                             { cs.leave(); }
};


//------------------------------------------------------------------------------


OutputComponentPtr Output::_output;
CriticalSectionPtr Output::_mutex;
HANDLE             Output::_event = NULL;
BOOL               Output::_reset = FALSE;

extern std::auto_ptr<XMPlayPlayer> player;


//------------------------------------------------------------------------------


INT_PTR Output::optionsDialogProc(HWND dialogWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	static std::auto_ptr<OptionsDialog> optionsDialog;

	switch (message)
	{
	case WM_INITDIALOG:
		if (player->majorVersion() == 3 && player->minorVersion() >= 5)
		{
			// area for options dialog was increased in v3.5 and again in v3.8
			const int dh = 24, dw = (player->minorVersion() < 8 ? 0 : 18);

			// adjust the size and position of the dialog items accordingly
			ResizeDlgItem(dialogWindow, DIALOG_OPTIONS_GROUP_BOX, dw, 0);
			ResizeDlgItem(dialogWindow, DIALOG_OPTIONS_CHILD_FRAME, dw, 0);
			RepositionDlgItem(dialogWindow, DIALOG_OPTIONS_APPLY_BUTTON, dw, dh);
		}

		// create plug-in options pane within player-provided dialog box
		optionsDialog.reset(new OptionsDialog);
		optionsDialog->setStatusCallback(
			// create callback that will update apply button
			std::tr1::bind(
				SetWindowEnabled,
				GetDlgItem(dialogWindow, DIALOG_OPTIONS_APPLY_BUTTON),
				std::tr1::placeholders::_1));
		optionsDialog->doModeless(
			GetDlgItem(dialogWindow, DIALOG_OPTIONS_CHILD_FRAME));
		break;

	case WM_DESTROY:
		optionsDialog.reset();
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case DIALOG_OPTIONS_APPLY_BUTTON:
				optionsDialog->doApply();
				break;
			}
		}
		break;
	}

	return 0;
}


const char* WINAPI Output::getName(DWORD face)
{
	return (face == 0 ? Plugin::name().c_str() : NULL);
}


DWORD WINAPI Output::getFlags(DWORD face)
{
	return (face == 0 ? XMPOUT_OUTPUT_VOLUME : -1);
}


BOOL WINAPI Output::open(DWORD face, XMPOUT_FORMAT* const form, HANDLE evnt)
{
	if (face == 0)
	{
		try
		{
			_mutex.reset(new CriticalSection);

			_output.reset(new OutputComponent(*player));
			_output->setProgressCallback(&Output::onBytesOutput);

			const OutputFormat format(SampleRate(form->form.rate),
				SampleSize(form->form.res), ChannelCount(form->form.chan));
			_output->open(format);

			_event = evnt;
			_reset = FALSE;

			return TRUE;
		}
		CATCH_ALL

		close();
	}

	return FALSE;
}


void WINAPI Output::close()
{
	try
	{
		OutputComponentPtr output(_output);
		if (output.get() != NULL)
		{
			try
			{
				output->close();
			}
			CATCH_ALL
		}
	}
	CATCH_ALL

	_event = NULL;
	_mutex.reset();
}


BOOL WINAPI Output::reset()
{
	// defer reset so new playback position can be determined
	return (_reset = TRUE);
}


BOOL WINAPI Output::write(const void* const buffer, DWORD length)
{
	try
	{
		ASSERT_NOT_NULL(_output);
		SYNC_BLOCK_WITH(*_mutex);

		const bool flag = _output->write((byte_t*) buffer, length);

		return (flag ? TRUE : FALSE);
	}
	CATCH_ALL

	return FALSE;
}


DWORD WINAPI Output::canWrite()
{
	try
	{
		ASSERT_NOT_NULL(_output);
		SYNC_BLOCK_WITH(*_mutex);

		if (_reset) // handle deferred reset before next write
		{
			_reset = FALSE;

			const time_t offset = player->getPlaybackPosition();
			_output->reset(offset);

			// seeking while paused eliminates separate call to resume
			_output->setPaused(false);
		}

		const size_t size = _output->canWrite();

		assert(size <= static_cast<size_t>(std::numeric_limits<DWORD>::max()));
		return static_cast<DWORD>(size);
	}
	CATCH_ALL

	return 0;
}


DWORD WINAPI Output::getBuffered()
{
	try
	{
		ASSERT_NOT_NULL(_output);
		SYNC_BLOCK_WITH(*_mutex);

		const size_t size = _output->buffered();

		assert(size <= static_cast<size_t>(std::numeric_limits<DWORD>::max()));
		return static_cast<DWORD>(size);
	}
	CATCH_ALL

	return 0;
}


BOOL WINAPI Output::setPaused(BOOL resume)
{
	try
	{
		ASSERT_NOT_NULL(_output);

		_output->setPaused(resume == FALSE);

		return TRUE;
	}
	CATCH_ALL

	return FALSE;
}


void WINAPI Output::setVolumeAndBalance(const float volume, const float balance)
{
	try
	{
		ASSERT_NOT_NULL(_output);

		float decibels;

		if (volume > 0.9999)
		{
			decibels = 0.0f;
		}
		else if (volume < 0.01)
		{
			decibels = -100.0f;
		}
		else
		{
			decibels = std::log10(volume) * 20.0f;
		}

		_output->setVolume(decibels);
		_output->setBalance(balance);
	}
	CATCH_ALL
}


void WINAPI Output::onNewTrack(const char* const filePath)
{
	try
	{
		ASSERT_NOT_NULL(_output);

		time_t length; shorts_t listpos; std::string title, album, artist;
		length = player->getPlaybackMetadata(listpos, title, album, artist);

		_output->setMetadata(OutputMetadata(length, title, album, artist, listpos));
	}
	CATCH_ALL
}


void Output::onBytesOutput(const size_t bytes)
{
	SetEvent(_event); // notify player
}
