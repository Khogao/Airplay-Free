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

#ifndef FoobarPlayer_h
#define FoobarPlayer_h


class FoobarPlayer
:
	public Player,
	private Uncopyable
{
public:
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

private:
	static void playImpl();
	static void pauseImpl();
	static void stopImpl();
	static void restartImpl();
	static void startNextImpl();
	static void startPrevImpl();
	static void increaseVolumeImpl();
	static void decreaseVolumeImpl();
	static void toggleMuteImpl();
	static void toggleShuffleImpl();
};


#endif // FoobarPlayer_h
