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
#include "FoobarPlayer.h"


HWND FoobarPlayer::window() const
{
	return core_api::get_main_window();
}


void FoobarPlayer::play()
{
	core_api::run_in_main_thread(playImpl);
}


void FoobarPlayer::pause()
{
	core_api::run_in_main_thread(pauseImpl);
}


void FoobarPlayer::stop()
{
	core_api::run_in_main_thread(stopImpl);
}


void FoobarPlayer::restart()
{
	core_api::run_in_main_thread(restartImpl);
}


void FoobarPlayer::startNext()
{
	core_api::run_in_main_thread(startNextImpl);
}


void FoobarPlayer::startPrev()
{
	core_api::run_in_main_thread(startPrevImpl);
}


void FoobarPlayer::increaseVolume()
{
	core_api::run_in_main_thread(increaseVolumeImpl);
}


void FoobarPlayer::decreaseVolume()
{
	core_api::run_in_main_thread(decreaseVolumeImpl);
}


void FoobarPlayer::toggleMute()
{
	core_api::run_in_main_thread(toggleMuteImpl);
}


void FoobarPlayer::toggleShuffle()
{
	core_api::run_in_main_thread(toggleShuffleImpl);
}

//------------------------------------------------------------------------------

void FoobarPlayer::playImpl()
{
	static_api_ptr_t<playback_control> player;

	player->start();
}


void FoobarPlayer::pauseImpl()
{
	static_api_ptr_t<playback_control> player;

	player->toggle_pause();
}


void FoobarPlayer::stopImpl()
{
	static_api_ptr_t<playback_control> player;

	player->stop();
}


void FoobarPlayer::restartImpl()
{
	static_api_ptr_t<playback_control> player;

	if (player->is_playing())
	{
		if (player->playback_can_seek())
		{
			player->playback_seek(0);
			player->pause(false);
		}
	}
	else
	{
		player->start();
	}
}


void FoobarPlayer::startNextImpl()
{
	static_api_ptr_t<playback_control> player;

	player->start(playback_control::track_command_next);
}


void FoobarPlayer::startPrevImpl()
{
	static_api_ptr_t<playback_control> player;

	player->start(playback_control::track_command_prev);
}


void FoobarPlayer::increaseVolumeImpl()
{
	static_api_ptr_t<playback_control> player;

	player->volume_up();
}


void FoobarPlayer::decreaseVolumeImpl()
{
	static_api_ptr_t<playback_control> player;

	player->volume_down();
}


void FoobarPlayer::toggleMuteImpl()
{
	static_api_ptr_t<playback_control> player;

	player->volume_mute_toggle();
}


void FoobarPlayer::toggleShuffleImpl()
{
	// don't know how to do this yet
}
