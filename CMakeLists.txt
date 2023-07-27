cmake_minimum_required(VERSION 3.14)
project(DWS LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS HttpServer)
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS Sql)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})



add_executable(DWS
  webserver.cpp
  tools/globalconfig.h tools/globalconfig.cpp
  server/globaldata.h server/globaldata.cpp
  server/globalevent.h server/globalevent.cpp
  server/globalcommon.h server/globalcommon.cpp
  server/webserver.h server/webserver.cpp
  server/database.h server/database.cpp
  tools/jsonwebtoken/src/qjsonwebtoken.h tools/jsonwebtoken/src/qjsonwebtoken.cpp 
  tools/bcrypt/qtbcrypt.h tools/bcrypt/qtbcrypt.cpp 
  tools/bcrypt/openwall_crypt/crypt_blowfish.h tools/bcrypt/openwall_crypt/crypt_blowfish.c 
  tools/bcrypt/openwall_crypt/crypt_gensalt.h tools/bcrypt/openwall_crypt/crypt_gensalt.c
  tools/bcrypt/openwall_crypt/crypt.h tools/bcrypt/openwall_crypt/ow-crypt.h tools/bcrypt/openwall_crypt/wrapper.c tools/bcrypt/openwall_crypt/x86.S
  tools/dockerconnector.h tools/dockerconnector.cpp
  tools/dockercontroller.h tools/dockercontroller.cpp
  tools/gpuinfo.h tools/gpuinfo.cpp
  )




target_link_libraries(DWS PRIVATE Qt6::Core Qt6::HttpServer crypt Qt6::Network Qt6::Sql Qt6::Concurrent yaml-cpp)



install(TARGETS DWS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable(DWSClient
  gpuclient.cpp
  client/globaldata.h client/globaldata.cpp
  client/globalevent.h client/globalevent.cpp
  tools/dockerconnector.h tools/dockerconnector.cpp
  tools/dockercontroller.h tools/dockercontroller.cpp
  tools/globalconfig.h tools/globalconfig.cpp
  client/globalcommon.h client/globalcommon.cpp
  tools/worker.hpp
  )

target_link_libraries(DWSClient PRIVATE Qt6::Core Qt6::HttpServer crypt Qt6::Network yaml-cpp ${CUDA_LIBRARIES})

install(TARGETS DWSClient
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})