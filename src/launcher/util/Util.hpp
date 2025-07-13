#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>

namespace util
{

//! Converts a narrow UTF8 string to a wide UTF16 string.
//!
//! @param narrowString Narrow string.
//! @returns Wide string.
std::wstring win32_widen(const std::string_view& narrowString);

//! Converts a wide UTF16 string to a narrow UTF8 string.
//!
//! @param wideString Wide string.
//! @returns Narrow string.
std::string win32_narrow(const std::wstring_view& wideString);

} // namespace util

#endif
