cmake_minimum_required(VERSION 3.14)

project(indexer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)

add_executable(indexer
  main.cpp
  indexer.h indexer.cpp
  dbhandler.h dbhandler.cpp
)
target_link_libraries(indexer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(indexer PRIVATE Qt${QT_VERSION_MAJOR}::Sql)

include(GNUInstallDirs)
install(TARGETS indexer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
