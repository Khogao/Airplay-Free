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

#ifndef XMPlayPlayer_h
#define XMPlayPlayer_h


#include "Player.h"
#include "Platform.h"
#include "Uncopyable.h"
#include <string>
#include <xmpfunc.h>


class XMPlayPlayer
:
	public Player,
	private Uncopyable
{
public:
	XMPlayPlayer(XMPFUNC_MISC*, XMPFUNC_STATUS*);

	BYTE majorVersion() const;
	BYTE minorVersion() const;
	HWND window() const;

	void play();
	void pause();
	void stop();
	void restart();
	void startNext();
	void startPrev();
	void increaseVolume();
	void decreaseVolume();
	void toggleMute();
	void toggleShuffle();

	time_t getPlaybackMetadata(shorts_t& listpos, std::string& title,
								std::string& album, std::string& artist) const;
	time_t getPlaybackPosition() const; // returns current track position in ms

private:
	DWORD_PTR sendIpcMessage(LPARAM, WPARAM = 0) const;

	XMPFUNC_MISC* const _miscFunc;
	XMPFUNC_STATUS* const _statusFunc;
};


#endif // XMPlayPlayer_h
