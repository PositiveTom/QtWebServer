cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-O0 -ggdb -fstack-protector-all)

option(ENABLE_APP "Enable data recorder" ON)

set(ALL_SRC "")
set(EXTERNAL_LIBRARIES_INCLUDE_DIRS "")
set(EXTERNAL_LIBRARIES "")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeHelper.cmake)

find_package(glog REQUIRED)
list(APPEND EXTERNAL_LIBRARIES glog::glog)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)
if(Qt5_FOUND)
    message(STATUS "Qt5 found")
    message(STATUS "Qt5_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}")
    message(STATUS "Qt5_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES}")
    list(APPEND EXTERNAL_LIBRARIES_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
    list(APPEND EXTERNAL_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES})
else()
    message(FATAL_ERROR "Qt5 not found")
endif()

# include dirs
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/ui/server
    ${EXTERNAL_LIBRARIES_INCLUDE_DIRS}
)

list(APPEND ALL_SRC
        src/ui/mainwindow/main_window.cpp
        src/ui/server/serverlogin.cpp
        src/ui/server/servermainwindow.cpp
        src/ui/server/servershowwindow.cpp
        src/threadpool/threadpool.cpp
        src/timer/timerwheel.cpp
        src/stream/stream.cpp
        src/stream/SocketStream.cpp
        src/stream/stream_line_reader.cpp
        src/server/HttpServer.cpp
        src/server/Server.cpp
        src/http/Request.cpp
)

QT_ADD_STATIC_LIB(${PROJECT_NAME} ${ALL_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC ${EXTERNAL_LIBRARIES})

if(ENABLE_APP)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_subdirectory(app)
endif()