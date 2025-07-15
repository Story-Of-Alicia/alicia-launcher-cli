#include "Alicia.hpp"

#include <liblauncher/util/Util.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <windows.h>
#include <winerror.h>

namespace
{

//! Settings.
struct Settings
{
  std::string _webInfoId;
  alicia::WebInfo _webInfoContent;
  std::string _executableProgram;
  std::string _executableArguments;
  bool _launch = false;
};

//! Loads settings from file.
//! @param path Path to settings file.
//! @param settings Reference to settings.
void load_settings(const std::string_view& path, Settings& settings)
{
  std::ifstream settingsFile(path.data());
  if (!settingsFile.is_open())
    throw std::runtime_error("The settings file does not exist.");

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
  catch (const nlohmann::json::exception& x)
  {
    throw x;
  }
}

} // namespace

int main(int argc, char** argv)
{
  std::string dir;
  dir.resize(512);
  dir.resize(GetCurrentDirectoryA(dir.size(),dir.data()));

  spdlog::info("Working directory: {}", dir);

  // Try to load the settings
  Settings settings;
  try
  {
    load_settings("settings.json", settings);
    spdlog::info("Loaded settings file");
  }
  catch (std::exception& x)
  {
    spdlog::error("Unhandled exception while loading the settings file: {}.", x.what());
    MessageBox(
      nullptr,
      "Couldn't load the settings file, is the launcher in the correct directory?",
      "Launcher",
      MB_OK | MB_ICONERROR);
    return 1;
  }

  // Try to parse the credentials from the CLI
  if (argc > 2)
  {
    settings._webInfoContent.loginId = argv[1];
    settings._webInfoContent.authKey = argv[2];

    spdlog::info("Credentials loaded from command-line");
  }

  // Host the web info
  alicia::WebInfoHost webInfoHost;
  try
  {
    webInfoHost.begin(settings._webInfoId, settings._webInfoContent);
    spdlog::info("Hosted the web info.");
  }
  catch (const std::exception& e)
  {
    spdlog::error("Failed to host web info: {}", e.what());
    MessageBox(nullptr, "Couldn't host the web info.", "Launcher", MB_OK | MB_ICONERROR);
    return 1;
  }

  // If launch is not set to true, do not spawn the game.
  if (!settings._launch)
  {
    spdlog::info("Not launching the game.");
    std::cin.ignore();
    return 0;
  }

  STARTUPINFO startupInfo{
    .cb = sizeof(STARTUPINFO)};
  PROCESS_INFORMATION processInfo{};

  spdlog::info("Launching the game ({} {})...",
    settings._executableProgram,
    settings._executableArguments);

  const BOOL result = CreateProcess(
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

  const auto lastWindowsError = GetLastError();
  if (result == FALSE && lastWindowsError != NO_ERROR)
  {
    if (lastWindowsError == ERROR_ELEVATION_REQUIRED)
    {
      spdlog::error("Can't launch the game, elevation is required");
      MessageBox(
        nullptr,
        "Couldn't launch the game, run the launcher as an administrator.",
        "Launcher",
        MB_OK | MB_ICONERROR);
    }
    if (lastWindowsError == ERROR_FILE_NOT_FOUND)
    {
      spdlog::error("Can't launch the game, the executable file was not found.");
      MessageBox(
        nullptr,
        "Couldn't launch the game, is the launcher in the working directory of the game?",
        "Launcher",
        MB_OK | MB_ICONERROR);
    }
    else
    {
      std::wstring errorMessageBuffer;
      errorMessageBuffer.resize(256);

      size_t const size = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        lastWindowsError,
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
        MB_OK | MB_ICONERROR);
    }

    spdlog::info("Waiting for input in order to exit (press ENTER).");
    std::cin.ignore();
  }
  else
  {
    spdlog::info("Game launched, idling until the process exits.");

    DWORD exitCode = 0;
    do
    {
      WaitForSingleObject(processInfo.hProcess, 500);
      GetExitCodeProcess(processInfo.hProcess, &exitCode);
    } while (exitCode == STILL_ACTIVE);

    spdlog::info("Game exited with code {}.", exitCode);
    return 0;
  }

  return 0;
}
