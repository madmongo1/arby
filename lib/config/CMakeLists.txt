#file(GLOB_RECURSE asioex_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")
file(GLOB_RECURSE arby_config_sources CONFIGURE_DEPENDS "*.hpp" "*.cpp")
add_library(arby_config ${arby_config_sources})
add_library(Arby::config ALIAS arby_config)
set_property(TARGET arby_config PROPERTY EXPORT_NAME config)
target_include_directories(arby_config PUBLIC ${lib_source_root})
if (ARBY_FIND_BOOST)
    set(ARBY_CONFIG_BOOST_LIBS Boost::filesystem Boost::system Boost::thread)
else()
    set(ARBY_CONFIG_BOOST_LIBS Boost::asio Boost::beast Boost::container Boost::filesystem Boost::json Boost::multiprecision Boost::scope_exit Boost::signals2 Boost::system Boost::thread)
endif()

target_link_libraries(arby_config PUBLIC
        ${ARBY_CONFIG_BOOST_LIBS}
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto
        WiseEnum::wise_enum
        fmt::fmt
        spdlog::spdlog)