cmake_minimum_required(VERSION 3.16)
project(code_translator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 components we need
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

qt_add_executable(code_translator
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(code_translator PRIVATE Qt6::Widgets Qt6::Network)
