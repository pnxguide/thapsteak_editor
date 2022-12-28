cmake_minimum_required(VERSION 3.25)
project(
    app
    VERSION 1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(wxWidgets_CONFIG_EXECUTABLE /Users/pnx/Documents/Project/Thapsteak/wxWidgets-3.2.1/build-cocoa-debug/wx-config)
find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})

find_package(fmt)

add_executable(app src/App.cpp)
include_directories(
    app
    PUBLIC
    src/include/)
target_link_libraries(app ${wxWidgets_LIBRARIES})
target_link_libraries(app fmt::fmt)
