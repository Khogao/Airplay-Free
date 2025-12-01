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

#include <foobar2000/SDK/abort_callback.cpp>
#include <foobar2000/SDK/advconfig.cpp>
#include <foobar2000/SDK/album_art.cpp>
#include <foobar2000/SDK/app_close_blocker.cpp>
#include <foobar2000/SDK/audio_chunk.cpp>
#include <foobar2000/SDK/audio_chunk_channel_config.cpp>
#include <foobar2000/SDK/cfg_var.cpp>
#include <foobar2000/SDK/chapterizer.cpp>
#include <foobar2000/SDK/commandline.cpp>
#include <foobar2000/SDK/completion_notify.cpp>
#include <foobar2000/SDK/config_object.cpp>
#include <foobar2000/SDK/console.cpp>
#include <foobar2000/SDK/dsp.cpp>
#include <foobar2000/SDK/dsp_manager.cpp>
#include <foobar2000/SDK/file_info.cpp>
#include <foobar2000/SDK/file_info_impl.cpp>
#include <foobar2000/SDK/file_info_merge.cpp>
#include <foobar2000/SDK/file_operation_callback.cpp>
#include <foobar2000/SDK/filesystem.cpp>
#include <foobar2000/SDK/filesystem_helper.cpp>
#include <foobar2000/SDK/guids.cpp>
#include <foobar2000/SDK/hasher_md5.cpp>
#include <foobar2000/SDK/input.cpp>
#include <foobar2000/SDK/input_file_type.cpp>
#include <foobar2000/SDK/link_resolver.cpp>
#include <foobar2000/SDK/mainmenu.cpp>
#include <foobar2000/SDK/mem_block_container.cpp>
#include <foobar2000/SDK/menu_helpers.cpp>
#include <foobar2000/SDK/menu_item.cpp>
#include <foobar2000/SDK/menu_manager.cpp>
#include <foobar2000/SDK/metadb.cpp>
#include <foobar2000/SDK/metadb_handle.cpp>
#include <foobar2000/SDK/metadb_handle_list.cpp>
#include <foobar2000/SDK/packet_decoder.cpp>
#include <foobar2000/SDK/playable_location.cpp>
#include <foobar2000/SDK/playback_control.cpp>
#include <foobar2000/SDK/playlist.cpp>
#include <foobar2000/SDK/playlist_loader.cpp>
#include <foobar2000/SDK/popup_message.cpp>
#include <foobar2000/SDK/preferences_page.cpp>
#include <foobar2000/SDK/replaygain.cpp>
#include <foobar2000/SDK/replaygain_info.cpp>
#include <foobar2000/SDK/service.cpp>
#include <foobar2000/SDK/tag_processor.cpp>
#include <foobar2000/SDK/tag_processor_id3v2.cpp>
#include <foobar2000/SDK/threaded_process.cpp>
#include <foobar2000/SDK/titleformat.cpp>
#include <foobar2000/SDK/ui.cpp>
#include <foobar2000/SDK/ui_element.cpp>

#include <foobar2000/helpers/clipboard.cpp>
#include <foobar2000/helpers/create_directory_helper.cpp>
#include <foobar2000/helpers/cue_creator.cpp>
#include <foobar2000/helpers/cue_parser.cpp>
#include <foobar2000/helpers/cue_parser_embedding.cpp>
#include <foobar2000/helpers/cuesheet_index_list.cpp>
#include <foobar2000/helpers/dialog_resize_helper.cpp>
#include <foobar2000/helpers/dropdown_helper.cpp>
#include <foobar2000/helpers/dynamic_bitrate_helper.cpp>
#include <foobar2000/helpers/file_info_const_impl.cpp>
#include <foobar2000/helpers/file_list_helper.cpp>
#include <foobar2000/helpers/file_move_helper.cpp>
#include <foobar2000/helpers/file_wrapper_simple.cpp>
//#include <foobar2000/helpers/filetimetools.cpp>
#include <foobar2000/helpers/IDataObjectUtils.cpp>
#include <foobar2000/helpers/input_helpers.cpp>
#include <foobar2000/helpers/listview_helper.cpp>
#include <foobar2000/helpers/metadb_io_hintlist.cpp>
#include <foobar2000/helpers/mp3_utils.cpp>
#include <foobar2000/helpers/seekabilizer.cpp>
#include <foobar2000/helpers/stream_buffer_helper.cpp>
#include <foobar2000/helpers/text_file_loader.cpp>
#include <foobar2000/helpers/VisUtils.cpp>
#include <foobar2000/helpers/wildcard.cpp>
#include <foobar2000/helpers/win32_dialog.cpp>
#include <foobar2000/helpers/win32_misc.cpp>
#include <foobar2000/helpers/window_placement_helper.cpp>

#include <foobar2000/ATLHelpers/AutoComplete.cpp>
#include <foobar2000/ATLHelpers/CDialogResizeHelper.cpp>
#include <foobar2000/ATLHelpers/inplace_edit.cpp>
#include <foobar2000/ATLHelpers/inplace_edit_v2.cpp>
#include <foobar2000/ATLHelpers/misc.cpp>

#include <pfc/base64.cpp>
#include <pfc/bsearch.cpp>
#include <pfc/guid.cpp>
#include <pfc/other.cpp>
#include <pfc/pathUtils.cpp>
#include <pfc/printf.cpp>
#include <pfc/profiler.cpp>
#include <pfc/selftest.cpp>
#include <pfc/sort.cpp>
#include <pfc/string.cpp>
#include <pfc/string_conv.cpp>
#include <pfc/stringNew.cpp>
#include <pfc/threads.cpp>
#include <pfc/utf8.cpp>
