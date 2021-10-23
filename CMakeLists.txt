cmake_minimum_required(VERSION 3.1)

project(DMC3_SRCNEXT)

include_directories(${CMAKE_SOURCE_DIR}/dependencies)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/imgui)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/minhook/include)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/rpclib/include)

include_directories($ENV{DXSDK_DIR}/Include)
link_directories("$ENV{DXSDK_DIR}/Lib/x86")

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:threadSafeInit- /MP")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

# Disable exceptions
# string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(Git)
if (Git_FOUND)
  execute_process(COMMAND
          "${GIT_EXECUTABLE}" log -1 --pretty=\"%h\"
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
          OUTPUT_VARIABLE GIT_COMMIT_HASH
          ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND
          "${GIT_EXECUTABLE}" log -1 --date short --pretty=\"%ad\"
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
          OUTPUT_VARIABLE GIT_COMMITTER_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(GIT_HASH ${GIT_COMMIT_HASH})
  set(GIT_DATE ${GIT_COMMITTER_DATE})
else()
  set(GIT_HASH "install_git_exe_bro")
  set(GIT_DATE "really")
endif()

set(LOG_FILENAME "${CMAKE_PROJECT_NAME}_framework_log.txt")
set(LOG_ENTRY "${CMAKE_PROJECT_NAME} ModFramework entry")
set(CONFIG_FILENAME "${CMAKE_PROJECT_NAME}_fw_config.txt")

configure_file(Config.hpp.in ${CMAKE_SOURCE_DIR}/src/Config.hpp @ONLY)
add_compile_options(/W0)

# Statically compile runtime
string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(dependencies)
add_subdirectory(src)