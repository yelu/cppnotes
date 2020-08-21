cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cppnotes)

# include conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(obj_model move_constructor.cpp)
target_include_directories(obj_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(file_aio file_aio.cpp)
target_link_libraries(file_aio PUBLIC aio pthread)

add_executable(socket_client socket_client.cpp)

add_executable(socket_server_blocking socket_server_blocking.cpp)

add_executable(socket_server_asio socket_server_asio.cpp)
target_link_libraries(socket_server_asio PUBLIC ${CONAN_LIBS})

add_executable(promise_future promise_future.cpp)
