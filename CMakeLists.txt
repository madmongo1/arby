cmake_minimum_required(VERSION 3.22)

include(FetchContent)

project(Arby VERSION 0.1)

find_package(Boost 1.78 REQUIRED COMPONENTS system thread filesystem)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        8.1.1
)
FetchContent_MakeAvailable(fmtlib)


add_subdirectory(arby)
