file(GLOB_RECURSE arby_util_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")

add_library(arby_util ${arby_util_srcs})
add_library(Arby::util ALIAS arby_util)
set_property(TARGET arby_util PROPERTY EXPORT_NAME util)
target_include_directories(arby_util PUBLIC ${lib_source_root})
