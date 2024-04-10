cmake_minimum_required(VERSION 3.5)

project(server_with_visual VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Network Sql)

set(PROJECT_SOURCES
        main.cpp
        administrator.cpp
        administrator.h
        administrator.ui
)


qt_add_executable(server_with_visual
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    message.h message.cpp message.ui

    registrationmodal.h registrationmodal.cpp registrationmodal.ui
    user.h user.cpp user.ui
)

target_link_libraries(server_with_visual PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt6::Network Qt6::Sql)


qt_finalize_executable(server_with_visual)

