#include "Alicia.hpp"
#include "util/Util.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <windows.h>
#include <winerror.h>

namespace {

//! Settings.
struct Settings {
  std::string _webInfoId;
  alicia::WebInfo _webInfoContent;
  std::string _executableProgram;
  std::string _executableArguments;
  bool _launch = true;
};

//! Loads settings from file.
//! @param path Path to settings file.
//! @param settings Reference to settings.
void load_settings(const std::string_view& path, Settings& settings)
{
  std::ifstream settingsFile(path.data());
  if(!settingsFile.is_open()) {
    throw std::runtime_error("The settings file does not exist.");
  }

  try
  {
    const auto json = nlohmann::json::parse(settingsFile);
    settings._webInfoId = json["webInfoId"];

    const nlohmann::json& webInfoContentJson = json["webInfoContent"];
    settings._webInfoContent = {
      .gameId = webInfoContentJson["GameId"],
      .memberNo = webInfoContentJson["MemberNo"],
      .loginId = webInfoContentJson["LoginId"],
      .authKey = webInfoContentJson["AuthKey"],
      .installUrl = webInfoContentJson["InstallUrl"],
      .serverType = webInfoContentJson["ServerType"],
      .serverInfo = webInfoContentJson["ServerInfo"],
      .age = webInfoContentJson["Age"],
      .sex = static_cast<alicia::WebInfo::Sex>(webInfoContentJson["Sex"]),
      .birthday = webInfoContentJson["Birthday"],
      .wardNo = webInfoContentJson["WardNo"],
      .cityCode = webInfoContentJson["CityCode"],
      .zipCode = webInfoContentJson["ZipCode"],
      .pcBangNo = webInfoContentJson["PcBangNo"],
      .closeTime = webInfoContentJson["CloseTime"],
    };

    settings._executableProgram = json["executableProgram"];
    settings._executableArguments = json["executableArguments"];
    settings._launch = json["launch"];
  }
  catch(const nlohmann::json::exception& x)
  {
    throw x;
  }
}

//! Registers the alicia launch protocol in the registry.
//! @param name name of the protocol
//! @param path absolute path to the protocol command executable
void register_protocol(const std::string& name, const std::string& path)
{
  HKEY protocol, command;
  int result;

  result = RegCreateKeyA(HKEY_CLASSES_ROOT, (LPCSTR)name.c_str(), &protocol);
  if(result != ERROR_SUCCESS)
    throw std::runtime_error("failed to create registry key HKEY_CLASSES_ROOT\\a2launch");

  result = RegSetValueEx(protocol, nullptr, 0, REG_SZ, (LPBYTE) "URL:a2launch Protocol", 22);
  if(result != ERROR_SUCCESS)
    throw std::runtime_error("failed to create registry value in 'HKEY_CLASSES_ROOT\\a2launch'");

  result = RegSetValueEx(protocol, "URL Protocol", 0, REG_SZ, nullptr, 0);
  if(result != ERROR_SUCCESS)
    throw std::runtime_error("failed to create registry value in 'HKEY_CLASSES_ROOT\\a2launch'");

  result = RegCreateKeyA(protocol, (LPCSTR) "shell\\open\\command", &command);
  if(result != ERROR_SUCCESS)
    throw std::runtime_error(
        "failed to create registry key 'HKEY_CLASSES_ROOT\\shell\\open\\command'");

  auto path_data = path.c_str();
  result = RegSetValueEx(command, nullptr, 0, REG_SZ, (LPBYTE)path_data, strlen(path_data));
  if(result != ERROR_SUCCESS)
    throw std::runtime_error("failed to create registry value for command in "
                             "'HKEY_CLASSES_ROOT\\a2launch\\shell\\open\\command'");
}

} // namespace

int main(int argc, char** argv)
{
  Settings settings;
  try
  {
    load_settings("settings.json", settings);
    spdlog::info("Loaded the settings.");
  }
  catch(std::exception& x)
  {
    spdlog::error("Failed to load the settings: {}.", x.what());
    MessageBox(nullptr, "Failed to load settings.", "Launcher", MB_OK);
    return 1;
  }

  alicia::WebInfoHost webInfoHost;
  try
  {
    webInfoHost.host(settings._webInfoId, settings._webInfoContent);
    spdlog::info("Hosted the web info.");
  }
  catch(const std::exception& e)
  {
    spdlog::error("Failed to host web info: {}", e.what());
    MessageBox(nullptr, "Couldn't host the web info.", "Launcher", MB_OK);
    return 0;
  }

  // If launch is not set to true, do not spawn the game.
  if (!settings._launch)
  {
    spdlog::info("Not launching the game. Idling in the background until input from console.");
    std::cin.ignore();
    return 0;
  }

  STARTUPINFO startupInfo{
    .cb = sizeof(STARTUPINFO)};
  PROCESS_INFORMATION processInfo{};

  spdlog::info("Launching the game ({} {})...",
    settings._executableProgram,
    settings._executableArguments);

  BOOL result = CreateProcess(
         settings._executableProgram.data(),
         settings._executableArguments.data(),
         nullptr,
         nullptr,
         FALSE,
         0,
         nullptr,
         nullptr,
         &startupInfo,
         &processInfo);

  if(result == FALSE && GetLastError() != NO_ERROR) {
    if (GetLastError() == ERROR_ELEVATION_REQUIRED) {
      spdlog::error("Can't launch the game, elevation is required");
      MessageBox(
          nullptr,
          "Couldn't launch the game, run the launcher as an administrator.",
          "Launcher",
          MB_OK);
    } else {
      std::wstring errorMessageBuffer;
      errorMessageBuffer.resize(256);

      size_t const size = FormatMessageW(
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          errorMessageBuffer.data(),
          errorMessageBuffer.size(),
          nullptr);
      errorMessageBuffer.resize(size);

      const std::string errorMessage = util::win32_narrow(
        errorMessageBuffer.substr(0, errorMessageBuffer.length()));

      spdlog::error(
        "Can't launch the game: {0}",
        errorMessage);

      MessageBox(
          nullptr,
          "Failed to launch the game, check the console window for more information.",
          "Launcher",
          MB_OK);
    }

    spdlog::info("Press ENTER to exit");
    std::cin.ignore();
  } else {
    spdlog::info("Game launched, idling until the process exits.");

    DWORD exitCode = 0;
    do {
      WaitForSingleObject(processInfo.hProcess, 1'000);

      GetExitCodeProcess(processInfo.hProcess, &exitCode);
      spdlog::info("Game exited with code {}.", exitCode);
    } while(exitCode == STILL_ACTIVE);
  }

  return 0;
}
