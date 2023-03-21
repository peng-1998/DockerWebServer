cmake_minimum_required(VERSION 3.14)

project(qtdockerwebserver LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS HttpServer)
add_executable(qtdockerwebserver
  main.cpp
  webserver.h
  webserver.cpp
  machinepool.h
  machinepool.cpp
  machine.h machine.cpp
  gpu.h gpu.cpp
)
target_link_libraries(qtdockerwebserver Qt6::Core Qt6::HttpServer crypt)

install(TARGETS qtdockerwebserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


