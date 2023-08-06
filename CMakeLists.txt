cmake_minimum_required(VERSION 3.27)
project(QtWebServer)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeHelper.cmake)

set(ALL_SRC "")
set(EXTERNAL_LIBRARIES_INCLUDE_DIRS "")
set(EXTERNAL_LIBRARIES "")

find_package(glog REQUIRED)
list(APPEND EXTERNAL_LIBRARIES glog::glog)

include_directories(
    src
    ${EXTERNAL_LIBRARIES_INCLUDE_DIRS}
)

list(APPEND ALL_SRC
        src/threadpool/threadpool.cpp
        src/timer/timerwheel.cpp
        src/stream/stream.cpp
        src/stream/bufferstream.cpp
        src/stream/socketstream.cpp
        src/stream/httpbuffer.cpp
        src/server/server.cpp
        src/server/selectserver.cpp
        src/utils/tool.cpp
)

QT_ADD_STATIC_LIB(${PROJECT_NAME} ${ALL_SRC})
target_link_libraries(${PROJECT_NAME} ${EXTERNAL_LIBRARIES}) 

add_subdirectory(app)