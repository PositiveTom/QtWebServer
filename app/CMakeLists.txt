add_executable(qtserver qtserver.cpp)
target_link_libraries(qtserver ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})


add_executable(test test.cpp)
target_link_libraries(qtserver ${EXTERNAL_LIBRARIES})
