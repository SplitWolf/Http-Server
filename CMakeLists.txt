cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(HttpServer VERSION 0.1.0)

add_executable(HttpServer 
              src/server.c
              src/winSockHelper.h
              src/winSockHelper.c)

if(WIN32)
  target_link_libraries(HttpServer wsock32 ws2_32)
endif()