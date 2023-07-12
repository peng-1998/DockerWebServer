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

add_executable(DWS
  webserver.cpp
  global/globalconfig.h global/globalconfig.cpp
  global/globaldata.h global/globaldata.cpp

  server/globalevent.h server/globalevent.cpp
  server/globalinit.h server/globalinit.cpp
  server/globalcommon.h server/globalcommon.cpp
  server/webserver.h server/webserver.cpp
  server/database.h server/database.cpp
  server/qjsonwebtoken.h server/qjsonwebtoken.cpp
)
target_link_libraries(DWS PRIVATE Qt6::Core Qt6::HttpServer crypt Qt6::Network Qt6::Sql Qt6::Concurrent)

install(TARGETS DWS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
