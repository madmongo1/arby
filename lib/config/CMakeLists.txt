#file(GLOB_RECURSE asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
file(GLOB_RECURSE arby_config_sources CONFIGURE_DEPENDS "*.hpp" "*.cpp")
add_library(arby_config ${arby_config_sources})
add_library(Arby::config ALIAS arby_config)
set_property(TARGET arby_config PROPERTY EXPORT_NAME config)
target_include_directories(arby_config PUBLIC ${lib_source_root})
target_link_libraries(arby_config PUBLIC
        Boost::system Boost::thread Boost::container
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto
        WiseEnum::wise_enum)
