#include "Alicia.hpp"

#include <format>
#include <iomanip>
#include <sstream>

namespace alicia
{

namespace
{

//! Serializes the web info into a string,
//! which then can be read by the game.
//!
//! @param webInfo Web info.
//! @returns Web info string.
std::string serialize_webinfo(const WebInfo& webInfo)
{
  std::stringstream outputStream;

  outputStream << "\t\t"
               << "|GameId=" << webInfo.gameId
               << "|MemberNo=" << webInfo.memberNo
               << "|LoginID=" << webInfo.loginId
               << "|AuthKey=" << webInfo.authKey
               << "|InstallUrl=" << webInfo.installUrl
               << "|ServerType=" << webInfo.serverType
               << "|ServerInfo=" << webInfo.serverInfo
               << "|Age=" << webInfo.age
               << "|Sex=" << static_cast<uint32_t>(webInfo.sex)
               << "|Birthday=" << webInfo.birthday
               << "|WardNo=" << webInfo.wardNo
               << "|CityCode="
               << std::setfill('0')
               << std::setw(2)
               << webInfo.cityCode
               << std::setw(0)
               << "|ZipCode=" << webInfo.zipCode
               << "|PCBangNo=" << webInfo.pcBangNo
               << "|CloseTime=" << webInfo.closeTime;

  return outputStream.str();
}

} // namespace

WebInfoHost::~WebInfoHost()
{
  destroy();
}

void WebInfoHost::begin(std::string webInfoId, WebInfo& webInfo)
{
  // Previous web info exists,
  // destroy it first.
  if (_webInfoMappingHandle)
  {
    destroy();
  }

  _webInfoId = std::move(webInfoId);
  _webInfo = webInfo;

  // Create new web info.
  create();
}

void WebInfoHost::end()
{
  destroy();
}

void WebInfoHost::create()
{
  // Create the shared file.
  DWORD error = 0;

  const auto tempDirectoryPath = std::filesystem::temp_directory_path() / "Alicia";
  create_directories(tempDirectoryPath);

  const auto webInfoFilePath = tempDirectoryPath / _webInfoId;

  _webInfoFileHandle = CreateFileW(
    webInfoFilePath.c_str(),
    GENERIC_READ | GENERIC_WRITE,
    0,
    nullptr,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
    nullptr);
  if (_webInfoFileHandle == INVALID_HANDLE_VALUE)
  {
    error = GetLastError();
    if (error)
    {
      throw std::runtime_error(
        std::format("Couldn't create file. Win32 Error 0x{:x}.", error));
    }
  }

  // Serialize the web info.
  const auto webInfo = serialize_webinfo(_webInfo);
  // Write the web info to the file.
  WriteFile(_webInfoFileHandle, webInfo.data(), webInfo.size(), nullptr, nullptr);
  FlushFileBuffers(_webInfoFileHandle);

  // Create a named file mapping.
  _webInfoMappingHandle = CreateFileMapping(
    _webInfoFileHandle, nullptr, PAGE_READWRITE, 0, 0, _webInfoId.data());
  error = GetLastError();
  if (error)
  {
    if (error == ERROR_FILE_INVALID)
    {
      throw std::runtime_error(
        std::format("Couldn't create a named file mapping. Invalid file.", error));
    }
    else
    {
      throw std::runtime_error(
        std::format("Couldn't create a named file mapping. Win32 Error 0x{:x}.", error));
    }
  }
}

void WebInfoHost::destroy()
{
  if (_webInfoMappingHandle)
  {
    UnmapViewOfFile(_webInfoMappingHandle);
    CloseHandle(_webInfoMappingHandle);
    _webInfoMappingHandle = nullptr;
  }

  if (_webInfoFileHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(_webInfoFileHandle);
    _webInfoFileHandle = nullptr;
  }
}

} // namespace alicia
