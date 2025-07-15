#include <liblauncher/util/Util.hpp>

#include <windows.h>

#include <iostream>
#include <format>
#include <filesystem>
#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

namespace
{

std::filesystem::path GetGameInstallDirectory()
{
  std::wstring pathBuffer;
  pathBuffer.resize(512);

  DWORD size = pathBuffer.size();
  RegGetValueW(HKEY_CURRENT_USER, L"Software\\Story of Alicia", nullptr, RRF_RT_REG_SZ, nullptr, pathBuffer.data(), &size);

  return {util::win32_narrow(pathBuffer)};
}

} // anon namespace

int main(int argc, char** argv)
{
  const std::filesystem::path installDirectory(
    GetGameInstallDirectory());
  if (installDirectory.empty())
  {
    spdlog::error("The game is not correctly installed, or not installed at all");
    std::cin.ignore();
    return 1;
  }

  SetCurrentDirectoryW(installDirectory.c_str());
  spdlog::info("Install directory: {}", installDirectory.string());

  std::vector<std::string> parameters;
  try
  {
    spdlog::info("Performing configuration");

    const std::string urlString = argv[1];
    auto url = util::parse_url(urlString);

    parameters.emplace_back(url.query["username"]);
    parameters.emplace_back(url.query["token"]);
  }
  catch (const std::exception& x)
  {
    spdlog::error(
      "Unhandled exception while performing configuration: {}",
      x.what());
    return 0;
  }

  const auto launcherPath = installDirectory / "alicia-launcher-cli.exe";

  std::wstring executableParameters;
  for (const auto& parameter : parameters)
  {
    executableParameters += util::win32_widen(parameter);
    executableParameters += L" ";
  }

  SHELLEXECUTEINFOW sei{
    sizeof(sei)};
  sei.lpVerb = L"runas";
  sei.lpFile = launcherPath.c_str();
  sei.lpParameters = executableParameters.c_str();
  sei.lpDirectory = launcherPath.parent_path().c_str();
  sei.nShow = SW_SHOWNORMAL;
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;

  if (not ShellExecuteExW(&sei)) {
    DWORD err = GetLastError();
    spdlog::error("Unhandled Win error while launching the launcher: {}", err);

    std::cin.ignore();
  }

  return 0;
}