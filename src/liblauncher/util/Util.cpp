#include "liblauncher/util/Util.hpp"

#include <stdexcept>

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
  const auto length = WideCharToMultiByte(
    CP_UTF8,
    0,
    wideString.data(),
    static_cast<int32_t>(wideString.size()),
    narrowString.data(),
    size,
    nullptr,
    nullptr);

  narrowString.resize(strlen(narrowString.c_str()));
  return narrowString;
}

Url parse_url(const std::string& urlString)
{
  if (urlString.empty())
    return {};

  const auto urlSchemaEndPosition = urlString.find("://");
  if (urlSchemaEndPosition == std::string::npos)
    throw std::runtime_error("Invalid URL, missing schema");

  const auto urlQueryStartPosition = urlString.find('?');

  Url url{
    .schema = urlString.substr(
      0, urlSchemaEndPosition),
    .path = urlString.substr(
      urlSchemaEndPosition + 3, urlQueryStartPosition - (urlSchemaEndPosition + 3))};

  if (urlQueryStartPosition == std::string::npos)
    return url;

  const std::string urlQueryString = urlString.substr(
    urlQueryStartPosition + 1);

  size_t offset = 0;
  while (true)
  {
    const auto endKeyVal = urlQueryString.find('&', offset);
    const std::string keyVal = urlQueryString.substr(offset, endKeyVal);

    const auto keyValMiddle = keyVal.find('=');
    url.query[keyVal.substr(0, keyValMiddle)] = keyVal.substr(keyValMiddle + 1);

    if (endKeyVal == std::string::npos)
      break;
    offset += endKeyVal + 1;
  }

  return url;
}

} // namespace util
