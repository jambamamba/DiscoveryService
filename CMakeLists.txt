#mkdir buildPi
#cd buildPi
#cmake -DCMAKE_TOOLCHAIN_FILE=/home/dev/oosman/pi/Toolchain-RaspberryPi.cmake ../
#make VERBOSE=1 -j8

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(EXEPROJ discovery.service)
SET (CMAKE_CXX_STANDARD_REQUIRED on)

project(${EXEPROJ})

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

include_directories (
    )
set (HEADERS
    DiscoveryData.h
    DiscoveryClient.h
    DiscoveryService.h
    SimpleUdpSocket.h
   )

set (SOURCES
   DiscoveryService.cpp
   DiscoveryClient.cpp
   SimpleUdpSocket.cpp
   main.cpp
   )

add_executable (${EXEPROJ}
    ${HEADERS}
    ${SOURCES}
   )

SET_TARGET_PROPERTIES(${EXEPROJ}
   PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

target_link_libraries (${EXEPROJ}
    pthread
   )
