add_executable(transcode main.cpp)

target_link_libraries(transcode ${Boost_LIBRARIES})

install (TARGETS transcode DESTINATION bin)