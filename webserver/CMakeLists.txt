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
find_package(Qt6 REQUIRED COMPONENTS Network)

add_executable(qtdockerwebserver
  main.cpp
  webserver.h webserver.cpp
  machinepool.h machinepool.cpp
  machine.h machine.cpp
  gpu.h gpu.cpp
  container.h container.cpp
  gpuwaitqueue.h gpuwaitqueue.cpp
  database.h database.cpp
  mailsender.h mailsender.cpp
)
target_link_libraries(qtdockerwebserver PRIVATE Qt6::Core Qt6::HttpServer crypt Qt6::Network)

install(TARGETS qtdockerwebserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


