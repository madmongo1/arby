file(GLOB_RECURSE arby_util_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
list(FILTER arby_util_srcs EXCLUDE REGEX "^.*\\.spec\\.[ch]pp$")
list(FILTER arby_util_srcs EXCLUDE REGEX "^.*/main\\.cpp$")

add_library(arby_util ${arby_util_srcs})
add_library(Arby::util ALIAS arby_util)
set_property(TARGET arby_util PROPERTY EXPORT_NAME util)
target_include_directories(arby_util PUBLIC ${lib_source_root})
target_link_libraries(arby_util PUBLIC Boost::boost Boost::thread Boost::system Arby::config)

file(GLOB_RECURSE arby_util_test_srcs CONFIGURE_DEPENDS "*.spec.hpp" "*.spec.cpp")
add_executable(arby_util_test ${arby_util_test_srcs})
target_link_libraries(arby_util_test PUBLIC Arby::util doctest::doctest)
add_test(NAME ArbyUtil COMMAND arby_util_test)
