file(GLOB_RECURSE arby_source_files CONFIGURE_DEPENDS "*.cpp" "*.hpp")
add_executable(arby ${arby_source_files})
target_include_directories(arby PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(arby PUBLIC
        Boost::system Boost::thread Boost::filesystem
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto
        fmt::fmt
        Arby::asioex Arby::util Arby::network Arby::config)
add_executable(Arby::arby ALIAS arby)
