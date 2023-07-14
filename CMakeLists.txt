cmake_minimum_required(VERSION 3.25)
project(
    app
    VERSION 1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(wxWidgets_CONFIG_EXECUTABLE /Users/pnx/Documents/Project/Thapsteak/wxWidgets-3.2.1/build-cocoa-debug/wx-config)
find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})

find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(fmt REQUIRED)

add_executable(app src/app.cpp src/notechart.cpp src/canvas.cpp)
include_directories(
    app
    PUBLIC
    src/include/)
include_directories(app PRIVATE third_party)

target_link_libraries(app ${wxWidgets_LIBRARIES})
target_link_libraries(app fmt::fmt)
target_link_libraries(app nlohmann_json::nlohmann_json)
