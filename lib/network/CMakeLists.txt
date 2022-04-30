file(GLOB_RECURSE arby_network_srcs CONFIGURE_DEPENDS "*.hpp" "*.cpp")

add_library(arby_network ${arby_network_srcs})
add_library(Arby::network ALIAS arby_network)
set_property(TARGET arby_network PROPERTY EXPORT_NAME util)
target_include_directories(arby_network PUBLIC ${lib_source_root})
target_link_libraries(arby_network PUBLIC Boost::thread Boost::system fmt::fmt Arby::config Arby::util)
