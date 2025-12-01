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

#ifndef Platform_h
#define Platform_h


// tag used on exported classes and functions
#if defined(_WIN32)
    #if defined(RSOUTPUT_EXPORTS)
    #	define RSOUTPUT_API __declspec(dllexport)
    #else
    #	define RSOUTPUT_API __declspec(dllimport)
    #endif
#else
    #define RSOUTPUT_API
#endif


#include <cstddef>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#else
#include <cstdint>
typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef uint8_t byte_t;
typedef char TCHAR;
typedef char char_t;
typedef void* HWND;
typedef int BOOL;
typedef long LONG;
typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT;
#define TEXT(quote) quote
#define MAKELONG(a, b) ((LONG)(((uint16_t)(((uintptr_t)(a)) & 0xffff)) | ((uint32_t)((uint16_t)(((uintptr_t)(b)) & 0xffff))) << 16))
#define LOWORD(l) ((uint16_t)(((uintptr_t)(l)) & 0xffff))
#define HIWORD(l) ((uint16_t)((((uintptr_t)(l)) >> 16) & 0xffff))
#endif

// int types
#ifdef _WIN32
typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef INT64 int64_t;
typedef UINT64 uint64_t;
#endif

// more types
using std::size_t;
using std::time_t;
#ifdef _WIN32
typedef UINT8 byte_t;
typedef TCHAR char_t;
#endif
typedef std::vector<byte_t> buffer_t;
typedef std::pair<short,short> shorts_t;
typedef std::basic_string<TCHAR> string_t;


namespace Platform {

	namespace Charset { // for string_t <-> std::string (with UTF-8)
		// std::string s1(...); string_t s2; Platform::Charset::fromUTF8(s1, s2);
		void fromUTF8(const std::string& src, std::string& dst);
		void fromUTF8(const std::string& src, std::wstring& dst);

		// string_t s1(TEXT(...)); std::string s2; Platform::Charset::toUTF8(s1, s2);
		void toUTF8(const std::string& src, std::string& dst);
		void toUTF8(const std::wstring& src, std::string& dst);
	}

	namespace Error {
		extern std::string describe(int errorCode /* from [WSA]GetLastError */);
#ifdef _WIN32
		inline std::string describeLast() { return describe(::GetLastError()); }
#else
        inline std::string describeLast() { return "Unknown Error"; }
#endif
	}

} // namespace Platform


//------------------------------------------------------------------------------
// simple global utility functions for dialogs and windows

extern "C" {

RSOUTPUT_API void CenterWindowOverParent(HWND);
RSOUTPUT_API void SetWindowEnabled(HWND, BOOL);

RSOUTPUT_API void ResizeDlgItem(HWND dlg, int itm, int dx, int dy);
RSOUTPUT_API void RepositionDlgItem(HWND dlg, int itm, int dx, int dy);

inline LONG WIDTH(const RECT& rect) { return (rect.right - rect.left); }
inline LONG HEIGHT(const RECT& rect) { return (rect.bottom - rect.top); }

} // extern "C"


#endif // Platform_h
