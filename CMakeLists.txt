cmake_minimum_required(VERSION 3.23)
project(alicia-launcher-cli
    LANGUAGES CXX
    DESCRIPTION "Command-line Alicia launcher"
    VERSION 1.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(3rd-party)

# project_properties
add_library(project_properties
        INTERFACE)
target_compile_features(project_properties
        INTERFACE cxx_std_20)

# liblauncher
add_library(alicia-liblauncher
        src/liblauncher/util/Util.cpp)
target_include_directories(alicia-liblauncher PUBLIC
        include)
target_link_libraries(alicia-liblauncher PUBLIC
        project_properties)

# launcher
add_executable(alicia-launcher-cli
        src/launcher/Alicia.cpp
        src/launcher/launcher.rc
        src/launcher/main.cpp)

target_link_libraries(alicia-launcher-cli PRIVATE
        alicia-liblauncher
        spdlog
        nlohmann_json)

if (${MSVC})
        target_compile_options(alicia-launcher-cli
                PRIVATE /utf-8)
        set_target_properties(alicia-launcher-cli PROPERTIES LINK_FLAGS
                "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")
endif ()

# launcher bridge
add_executable(alicia-launcher-bridge
        src/launcher-bridge/launcher.rc
        src/launcher-bridge/main.cpp)
target_link_libraries(alicia-launcher-bridge PRIVATE
        alicia-liblauncher
        spdlog)

if (${MSVC})
        target_compile_options(alicia-launcher-bridge
                PRIVATE /utf-8)
        set_target_properties(alicia-launcher-bridge PROPERTIES LINK_FLAGS
                "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")
endif ()

# Copy the resources
add_custom_command(
        TARGET alicia-launcher-cli POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CC_COMPILER_ID STREQUAL "GNU")
        target_link_options(alicia-launcher-cli PRIVATE -static)
endif()