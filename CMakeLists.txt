#mkdir buildPi
#cd buildPi
#cmake -DCMAKE_TOOLCHAIN_FILE=/home/dev/oosman/pi/Toolchain-RaspberryPi.cmake ../
#make VERBOSE=1 -j8

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(EXEPROJ discovery.service)
set(LIBPROJ discovery.lib)
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
    Utils.h
   )

set (SOURCES
   DiscoveryService.cpp
   DiscoveryClient.cpp
   SimpleUdpSocket.cpp
   Utils.cpp
   )

add_executable (${EXEPROJ}
    ${HEADERS}
    ${SOURCES}
    main.cpp
   )
add_library (${LIBPROJ}
    ${HEADERS}
    ${SOURCES}
   )
SET_TARGET_PROPERTIES(${EXEPROJ}
   PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")
SET_TARGET_PROPERTIES(${LIBPROJ}
   PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

target_link_libraries (${EXEPROJ}
    pthread
   )
target_link_libraries (${LIBPROJ}
    pthread
   )
