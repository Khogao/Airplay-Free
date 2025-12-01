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

class Output : public dsp_impl_base
{
public:
	static GUID g_get_guid();
	static void g_get_name(pfc::string_base&);
	static bool g_get_default_preset(dsp_preset&);
	static bool g_have_config_popup() { return true; }
	static void g_show_config_popup(const dsp_preset&, HWND, dsp_preset_edit_callback&);

public:
	Output(const dsp_preset&) {}

	void flush() {};
	double get_latency();
	bool need_track_change_mark() { return true; }

protected:
	bool on_chunk(audio_chunk*, abort_callback&);
	void on_endofplayback(abort_callback&) {}
	void on_endoftrack(abort_callback&);

private:
	OutputMetadata to_metadata(const metadb_handle_ptr&) const;
};

static const dsp_factory_t<Output> dsp_factory_g;

//------------------------------------------------------------------------------

class Playback : public play_callback_static
{
public:
	unsigned get_flags() {
		on_volume_change(static_api_ptr_t<playback_control>()->get_volume()); // pass along initial volume
		return flag_on_playback_pause | flag_on_playback_seek | flag_on_playback_stop | flag_on_volume_change;
	}

	void on_playback_stop(play_control::t_stop_reason reason) { // remap stop callback
		switch (reason) {
		case play_control::stop_reason_starting_another:
			on_playback_seek(0);
			on_playback_pause(false);
			break;
		default:
			on_playback_stop();
		}
	}

	void on_playback_pause(bool);
	void on_playback_seek(double);
	void on_playback_stop(void);
	void on_volume_change(float);

	void on_playback_starting(play_control::t_track_command, bool) {}
	void on_playback_new_track(metadb_handle_ptr) {}
	void on_playback_edited(metadb_handle_ptr) {}
	void on_playback_dynamic_info(const file_info&) {}
	void on_playback_dynamic_info_track(const file_info&) {}
	void on_playback_time(double) {}
};

static const play_callback_static_factory_t<Playback> play_callback_static_factory_g;

//------------------------------------------------------------------------------

GUID Output::g_get_guid()
{
	static const GUID guid = { Plugin::id(), 0, 0, 0 };

	return guid;
}


void Output::g_get_name(pfc::string_base& name)
{
	name = Plugin::name().c_str();
}


bool Output::g_get_default_preset(dsp_preset& preset)
{
	preset.set_data(0, 0);
	preset.set_owner(g_get_guid());

	return true;
}


void Output::g_show_config_popup(const dsp_preset&, HWND parent, dsp_preset_edit_callback&)
{
	Options::show_popup(parent);
}

//------------------------------------------------------------------------------

#define ASSERT_NOT_NULL(ptr) if ((ptr).get() == NULL) throw (#ptr " is null")
#define SYNC_BLOCK_WITH(mtx) insync(mtx)

namespace {
	typedef std::auto_ptr<OutputComponent> OutputComponentPtr;
	typedef critical_section OutputMutex;
}

extern OutputComponentPtr output;
static OutputFormat       format;
static OutputMutex        mutex;
static bool resume = false;


double Output::get_latency()
{
	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		// convert buffered output from bytes to seconds
		const double latency = double(output->buffered()) / double(
			format.sampleRate() * format.sampleSize() * format.channelCount());

		return latency;
	}
	CATCH_ALL

	return 0;
}


bool Output::on_chunk(audio_chunk* const chunk, abort_callback&)
{
	using namespace std::rel_ops;
	static const uint8_t bps = 16;
	static metadb_handle* ptr = NULL;

	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		// reopen on format change
		const OutputFormat fmt(SampleRate(chunk->get_sample_rate()),
			SampleSize(bps / 8), ChannelCount(chunk->get_channel_count()));
		if (fmt != format)
		{
			ptr = NULL;
			format = fmt;
			output->open(format);
		}
		else if (resume)
		{
			output->setPaused(false);
		}
		resume = false;

		// update metadata on track change
		metadb_handle_ptr cur_file;
		if (get_cur_file(cur_file) && cur_file.get_ptr() != ptr)
		{
			ptr = cur_file.get_ptr();
			output->setMetadata(to_metadata(cur_file));
		}

		mem_block_container_impl buf;
		if (!chunk->to_raw_data(buf, bps)
			|| output->canWrite() < buf.get_size()
			|| !output->write((const byte_t*) buf.get_ptr(), buf.get_size()))
		{
			Debugger::printf("Unable to write %lu bytes to output", buf.get_size());
		}
	}
	CATCH_ALL

	return true;
}


void Output::on_endoftrack(abort_callback&)
{
	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		output->write(NULL, 0);
	}
	CATCH_ALL
}


void Playback::on_playback_pause(const bool pause)
{
	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		if (pause)
		{
			output->setPaused(true);
		}
		else
		{
			resume = true;
		}
	}
	CATCH_ALL
}


void Playback::on_playback_seek(const double offset)
{
	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		// convert offset from seconds to milliseconds
		output->reset(time_t(offset * 1000.0));
	}
	CATCH_ALL
}


void Playback::on_playback_stop()
{
	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		output->close();
		format = OutputFormat();
	}
	CATCH_ALL
}


void Playback::on_volume_change(const float volume)
{
	try
	{
		ASSERT_NOT_NULL(output);
		SYNC_BLOCK_WITH(mutex);

		output->setVolume(std::min(std::max(volume, -100.0f), 0.0f));
	}
	CATCH_ALL
}


static void get_listpos(shorts_t& listpos, const metadb_handle_ptr& mdb,
	win32_event& complete)
{
	try
	{
		t_size idx, len, ppl; metadb_handle_ptr itm;
		const static_api_ptr_t<playlist_manager> plm;

		// plm->get_playing_item_location() is the simplest means to determine
		// the playlist and position, but it fails in some cases when skipping
		// around and returns the last played item, so double check its result
		if (plm->get_playing_item_location(&ppl, &idx)
			&& ppl == plm->get_playing_playlist() //alt api
			&& (len = plm->playlist_get_item_count(ppl)) > 0)
		{
			// compare item at given index to known good mdb pointer
			if (itm = plm->playlist_get_item_handle(ppl, idx),
				itm.is_valid() && itm == mdb)
			{
				listpos = std::make_pair(idx + 1, len);
				goto end;
			}
			// try next item in the playlist (assumes normal play order)
			else if (itm = plm->playlist_get_item_handle(ppl, idx + 1),
				itm.is_valid() && itm == mdb)
			{
				listpos = std::make_pair(idx + 2, len);
				goto end;
			}
		}

		if ((ppl = plm->get_playing_playlist()) != -1
			&& (len = plm->playlist_get_item_count(ppl)) > 0)
		{
			// scan the playlist for first match to mdb pointer
			// NOTE: this does not account for duplicate entries
			for (idx = 0; idx < len && complete.is_valid(); ++idx)
			{
				itm = plm->playlist_get_item_handle(ppl, idx);
				if (itm.is_valid() && itm == mdb)
				{
					if (complete.is_valid())
						listpos = std::make_pair(idx + 1, len);
					break;
				}
			}
		}
	}
	CATCH_ALL

end:
	if (complete.is_valid()) complete.set_state(true);
}


OutputMetadata Output::to_metadata(const metadb_handle_ptr& mdb) const
{
	std::string title;
	std::string album;
	std::string artist;
	time_t      length(0);
	shorts_t    listpos(0,0);
	buffer_t    artworkData(0);
	std::string artworkType("image/none");

	win32_event complete; complete.create(true, false);
	core_api::run_in_main_thread(std::tr1::bind(get_listpos,
		std::tr1::ref(listpos), std::tr1::ref(mdb), std::tr1::ref(complete)));

	const file_info* inf;
	if (mdb->get_info_locked(inf))
	{
		length = time_t(inf->get_length() * 1000.0);
		if (inf->meta_exists("title")) title = inf->meta_get("title", 0);
		if (inf->meta_exists("album")) album = inf->meta_get("album", 0);
		if (inf->meta_exists("artist")) artist = inf->meta_get("artist", 0);
	}

	if (title.empty())
	{
		const char* const path = mdb->get_path();  char file[MAX_PATH];
		title = (_splitpath_s(path, NULL, 0, NULL, 0, file, sizeof(file), NULL, 0) ? path : file);
	}

	try
	{
		const static_api_ptr_t<album_art_manager_v2> aam;

		const album_art_extractor_instance_v2::ptr aae = aam->open(
			pfc::list_t<metadb_handle_ptr>() += mdb,
			pfc::list_t<GUID>() += album_art_ids::cover_front,
			abort_callback_dummy());

		const album_art_data_ptr aad = aae->query(
			album_art_ids::cover_front, abort_callback_dummy());

		if (aad.get_ptr() != NULL && aad->get_size() > 10
			&& std::memcmp((const byte_t*) aad->get_ptr() + 6, "JFIF", 4) == 0)
		{
			artworkType = "image/jpeg";
			artworkData.resize(aad->get_size());
			std::memcpy(&artworkData[0], aad->get_ptr(), aad->get_size());
		}
	}
	catch (const exception_album_art_not_found&)
	{
	}
	CATCH_ALL

	complete.wait_for(0.5); complete.release();

	return OutputMetadata(length, title, album, artist, listpos, artworkData, artworkType);
}
