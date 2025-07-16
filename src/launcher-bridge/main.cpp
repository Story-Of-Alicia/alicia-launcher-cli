#include <liblauncher/util/Util.hpp>

#include <windows.h>

#include <format>
#include <filesystem>
#include <string>

#include <spdlog/spdlog.h>

namespace
{

//! Launcher executable name.
constexpr std::wstring_view LauncherExecutableName = L"alicia-launcher-cli.exe";

//! Get the game install directory from the registry.
//! @returns The path to the game install directory.
std::filesystem::path GetGameInstallDirectory()
{
  std::wstring pathBuffer;
  pathBuffer.resize(512);

  DWORD size = pathBuffer.size();
  RegGetValueW(
    HKEY_CURRENT_USER,
    L"Software\\Story of Alicia",
    nullptr,
    RRF_RT_REG_SZ,
    nullptr,
    pathBuffer.data(),
    &size);

  return pathBuffer;
}

} // anon namespace

int main(int argc, char** argv)
{
  // Get the game install directory and validate it
  const std::filesystem::path installDirectory = GetGameInstallDirectory();
  spdlog::info("Game install directory: {}", installDirectory.string());

  if (installDirectory.empty()
    || not std::filesystem::exists(installDirectory))
  {
    spdlog::error("The game install directory is empty or does not exist.");
    MessageBox(
      nullptr,
      "The game install directory is empty or does not exist. "
      "The game is not correctly installed.",
      "Launcher Bridge",
      MB_OK | MB_ICONERROR);
    return 1;
  }

  // Set the working directory to the install directory
  SetCurrentDirectoryW(installDirectory.c_str());

  // Parse the launch URI
  std::vector<std::string> credentials;
  try
  {
    const std::string uriString = argv[1];
    auto uri = util::ParseUri(uriString);

    credentials.emplace_back(uri.path);
    credentials.emplace_back(uri.query["username"]);
    credentials.emplace_back(uri.query["token"]);

    spdlog::info("Parsed the launch URI");
  }
  catch (const std::exception& x)
  {
    spdlog::error("Failed to parse the launch URI: {}", x.what());
    MessageBox(
      nullptr,
      "Failed to parse the launch URI. Check the console.",
      "Launcher Bridge",
      MB_OK | MB_ICONERROR);
    return 0;
  }

  // Build the parameters for the executable
  std::wstring parameters;
  for (const auto& parameter : credentials)
  {
    parameters += util::win32_widen(parameter);
    parameters += L" ";
  }

  SHELLEXECUTEINFOW shellExecuteCtx{
    .cbSize = sizeof(shellExecuteCtx),
    .fMask = SEE_MASK_NOCLOSEPROCESS,
    .lpVerb = L"runas",
    .lpFile = LauncherExecutableName.data(),
    .lpParameters = parameters.c_str(),
    .lpDirectory = installDirectory.c_str(),
    .nShow = SW_SHOWNORMAL,};

  if (not ShellExecuteExW(&shellExecuteCtx)) {
    const DWORD err = GetLastError();
    spdlog::error("Failed to bridge to the launcher: Windows error {}", err);
    MessageBox(
      nullptr,
      "Failed to start the launcher. Check the console.",
      "Launcher Bridge",
      MB_OK | MB_ICONERROR);
  }

  return 0;
}