#include "Platform.h"
#include <string>
#include <vector>
#include <iostream>
#include <cstring>

// Linux implementation of Platform.h functions
// The original code used Windows-specific APIs (MessageBox, Registry, etc.)
// We need to replace them with Linux equivalents (Qt, config files, etc.)

namespace Platform {

namespace Charset {
    void fromUTF8(const std::string& src, std::string& dst) { dst = src; }
    void fromUTF8(const std::string& src, std::wstring& dst) { /* TODO: mbstowcs */ }
    void toUTF8(const std::string& src, std::string& dst) { dst = src; }
    void toUTF8(const std::wstring& src, std::string& dst) { /* TODO: wcstombs */ }
}

namespace Error {
    std::string describe(int errorCode) { return "Unknown Error"; }
}

} // namespace Platform

// Global utility functions implementation for Linux
extern "C" {

void CenterWindowOverParent(HWND) {
    // No-op on Linux/Qt for now
}

void SetWindowEnabled(HWND, BOOL) {
    // No-op
}

void ResizeDlgItem(HWND, int, int, int) {}
void RepositionDlgItem(HWND, int, int, int) {}

}
