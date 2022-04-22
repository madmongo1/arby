#file(GLOB_RECURSE asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
add_library(arby_config INTERFACE)
add_library(Arby::config ALIAS arby_config)
set_property(TARGET arby_config PROPERTY EXPORT_NAME config)
target_include_directories(arby_config INTERFACE ${lib_source_root})
target_link_libraries(arby_config INTERFACE
        Boost::system Boost::thread
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto)
