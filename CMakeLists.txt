cmake_minimum_required(VERSION 3.22)

include(FetchContent)

project(Arby VERSION 0.1)

find_package(Boost 1.79 REQUIRED COMPONENTS system thread filesystem container)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        8.1.1
)
FetchContent_MakeAvailable(fmtlib)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.10.0
)
set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  wise_enum_dep
  GIT_REPOSITORY https://github.com/quicknir/wise_enum.git
  GIT_TAG        master
)
#FetchContent_MakeAvailable(wise_enum_dep)

if(NOT wise_enum_dep_POPULATED)
    FetchContent_Populate(wise_enum_dep)
    add_subdirectory(${wise_enum_dep_SOURCE_DIR})
    target_include_directories(wise_enum INTERFACE "$<BUILD_INTERFACE:${wise_enum_dep_SOURCE_DIR}>")
endif()



add_subdirectory(lib)
add_subdirectory(arby)
