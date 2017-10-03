cmake_minimum_required(VERSION 3.8)
project(ipTest)

set(CMAKE_CXX_STANDARD 11)

find_package( OpenCV REQUIRED )

add_executable( main main.cpp )
target_link_libraries( main ${OpenCV_LIBS} )