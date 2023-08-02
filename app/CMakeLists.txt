# add_executable(demo_ui ${CMAKE_CURRENT_SOURCE_DIR}/ui/demo_ui.cpp)
# target_link_libraries(demo_ui PUBLIC ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})


# add_executable(server ${CMAKE_CURRENT_SOURCE_DIR}/server/server.cpp)
# target_link_libraries(server PUBLIC ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})


# add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/client/client.cpp)
# target_link_libraries(client PUBLIC ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})


add_executable(timer ${CMAKE_CURRENT_SOURCE_DIR}/timer/timer.cpp)
target_link_libraries(timer PUBLIC ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})


add_executable(threadpool-test threadpool/threadpool-test.cpp)
target_link_libraries(threadpool-test PUBLIC ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})


add_executable(QtHttpServer QtHttpServer.cpp)
target_link_libraries(QtHttpServer PUBLIC ${PROJECT_NAME} ${EXTERNAL_LIBRARIES})
