cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cppnotes)

add_executable(obj_model move_constructor.cpp)
target_include_directories(obj_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(file_aio file_aio.cpp)
target_link_libraries(file_aio PUBLIC aio pthread)

add_executable(net_sync_io echo_server_sync.cpp)
