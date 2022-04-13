
add_executable(arby main.cpp)
target_link_libraries(arby PUBLIC Boost::system Boost::thread Threads::Threads OpenSSL::SSL OpenSSL::Crypto fmt::fmt Boost::filesystem)
add_executable(Arby::arby ALIAS arby)
