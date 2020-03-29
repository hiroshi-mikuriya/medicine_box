cmake_minimum_required(VERSION 3.6)
project(medicine_box)
  
set(CMAKE_CXX_STANDARD 11)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message( "Build for Linux with g++" )
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_CXX_FLAGS "-std=c++11 -ffunction-sections -O3 -Wall -Wsign-compare -Wconversion")
    set(LIBUSB_INC /usr/local/include)
    set(LIBUSB /usr/local/lib/libusb-1.0.so -latomic)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message( "Build for Darwin with clang++" )
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_CXX_FLAGS "-std=c++11 -ffunction-sections -O3 -Wall -Wsign-compare -Wconversion")
    set(LIBUSB_INC /usr/local/include)
    set(LIBUSB /usr/local/lib/libusb-1.0.dylib)
else()
    message( "unknown platform" )
    return(1)
endif()

find_package(Threads REQUIRED)
 
file(GLOB SOURCES
    *.cpp
    *.c
)

include_directories(
    .
    ${LIBUSB_INC}
)
 
add_executable(
    ${CMAKE_PROJECT_NAME}
    ${SOURCES}
)
 
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBUSB}
)