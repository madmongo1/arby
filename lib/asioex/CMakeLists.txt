file(GLOB_RECURSE arby_asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
list(FILTER arby_asioex_srcs EXCLUDE REGEX "^.*\\.spec\\.[ch]pp$")
list(FILTER arby_asioex_srcs EXCLUDE REGEX "^.*/main\\.cpp$")

add_library(arby_asioex ${arby_asioex_srcs})
add_library(Arby::asioex ALIAS arby_asioex)
set_property(TARGET arby_asioex PROPERTY EXPORT_NAME asioex)
target_include_directories(arby_asioex PUBLIC ${lib_source_root})
target_link_libraries(arby_asioex PUBLIC
        Boost::system Boost::thread
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto Arby::config)

file(GLOB_RECURSE arby_asioex_test_srcs CONFIGURE_DEPENDS "*.spec.hpp" "*.spec.cpp")
add_executable(arby_asioex_test ${arby_asioex_test_srcs})
target_link_libraries(arby_asioex_test PUBLIC Arby::asioex doctest::doctest)
add_test(NAME ArbyAsioex COMMAND arby_asioex_test)

#file(GLOB_RECURSE asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
