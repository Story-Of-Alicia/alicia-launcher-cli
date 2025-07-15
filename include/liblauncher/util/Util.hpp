#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

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

struct Url
{
  std::string schema;
  std::string path;
  std::unordered_map<std::string, std::string> query;
};

Url parse_url(const std::string& urlString);

} // namespace util

#endif
