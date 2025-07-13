#include "Util.hpp"

#include <windows.h>

namespace util
{

std::wstring win32_widen(const std::string_view& narrowString)
{
  if (narrowString.empty())
    return L"";

  std::wstring wideString;
  wideString.resize(narrowString.size());
  MultiByteToWideChar(
    CP_UTF8,
    0,
    narrowString.data(),
    static_cast<int32_t>(narrowString.size()),
    wideString.data(),
    static_cast<int32_t>(wideString.size()));
  return wideString;
}

std::string win32_narrow(const std::wstring_view& wideString)
{
  if (wideString.empty())
    return "";

  // Determine the required size of the narrow UTF8 string.
  const auto size = WideCharToMultiByte(
    CP_UTF8,
    0,
    wideString.data(),
    static_cast<int32_t>(wideString.size()),
    nullptr,
    0,
    nullptr,
    nullptr);

  // Convert the unicode string to UTF8 string.
  std::string narrowString;
  narrowString.resize(size);
  WideCharToMultiByte(
    CP_UTF8,
    0,
    wideString.data(),
    static_cast<int32_t>(wideString.size()),
    narrowString.data(),
    size,
    nullptr,
    nullptr);

  return narrowString;
}

} // namespace util