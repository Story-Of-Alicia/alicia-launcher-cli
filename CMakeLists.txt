cmake_minimum_required(VERSION 3.23)
project(alicia-launcher-cli
    LANGUAGES CXX
    DESCRIPTION "Command-line Alicia launcher"
    VERSION 1.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(3rd-party)

add_library(project_properties
        INTERFACE)
target_compile_features(project_properties
        INTERFACE cxx_std_20)

add_executable(alicia-launcher-cli
        src/launcher/util/Util.cpp
        src/launcher/Alicia.cpp
        src/launcher/launcher.rc
        src/launcher/main.cpp)

# Copy the resources
add_custom_command(
        TARGET alicia-launcher-cli POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR})

if (${MSVC})
        target_compile_options(alicia-launcher-cli
                PRIVATE /utf-8)
endif ()

target_link_libraries(alicia-launcher-cli
        PUBLIC project_properties
        PRIVATE spdlog nlohmann_json)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CC_COMPILER_ID STREQUAL "GNU")
        target_link_options(alicia-launcher-cli PRIVATE -static)
endif()