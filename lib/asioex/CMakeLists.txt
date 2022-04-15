#file(GLOB_RECURSE asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
add_library(asioex INTERFACE)
message(STATUS "lib_source_root=${lib_source_root}")
target_include_directories(asioex INTERFACE ${lib_source_root})
