cmake_minimum_required(VERSION 3.14)

project(DWS LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS HttpServer)
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS Sql)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(DWS
  webserver.cpp
  server/globalconfig.h server/globalconfig.cpp
  server/globaldata.h server/globaldata.cpp
  server/globalevent.h server/globalevent.cpp
  server/globalinit.h server/globalinit.cpp
  server/globalcommon.h server/globalcommon.cpp
  server/webserver.h server/webserver.cpp
  server/database.h server/database.cpp
  server/jsonwebtoken/src/qjsonwebtoken.h server/jsonwebtoken/src/qjsonwebtoken.cpp 
  server/bcrypt/qtbcrypt.h server/bcrypt/qtbcrypt.cpp 
  server/bcrypt/openwall_crypt/crypt_blowfish.h server/bcrypt/openwall_crypt/crypt_blowfish.c 
  server/bcrypt/openwall_crypt/crypt_gensalt.h server/bcrypt/openwall_crypt/crypt_gensalt.c
  server/bcrypt/openwall_crypt/crypt.h server/bcrypt/openwall_crypt/ow-crypt.h server/bcrypt/openwall_crypt/wrapper.c server/bcrypt/openwall_crypt/x86.S
  tools/dockerconnector.h tools/dockerconnector.cpp
  tools/dockercontroller.h tools/dockercontroller.cpp
  tools/gpuinfo.h tools/gpuinfo.cpp
  )
target_link_libraries(DWS PRIVATE Qt6::Core Qt6::HttpServer crypt Qt6::Network Qt6::Sql Qt6::Concurrent yaml-cpp)

install(TARGETS DWS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
