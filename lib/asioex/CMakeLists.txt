#file(GLOB_RECURSE asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
add_library(arby_asioex INTERFACE)
add_library(Arby::asioex ALIAS arby_asioex)
set_property(TARGET arby_asioex PROPERTY EXPORT_NAME asioex)
target_include_directories(arby_asioex INTERFACE ${lib_source_root})
target_link_libraries(arby_asioex INTERFACE
        Boost::system Boost::thread
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto)
