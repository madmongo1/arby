
file(GLOB scratch_cpps CONFIGURE_DEPENDS "*.cpp")
set(scratch_required_libs Boost::system Boost::thread Boost::filesystem
        Threads::Threads
        OpenSSL::SSL OpenSSL::Crypto
        Arby::asioex Arby::util Arby::network Arby::config)

foreach(scratch_cpp IN LISTS scratch_cpps)
    get_filename_component(export_name ${scratch_cpp} NAME_WLE)
    set(target_name "arby_${export_name}")
    add_executable(${target_name} ${scratch_cpp})
    target_include_directories(${target_name} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${target_name} PUBLIC ${scratch_required_libs})
    add_executable("Arby::${export_name}" ALIAS ${target_name})
endforeach()





