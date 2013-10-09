add_executable(generate_nfa main.cpp)

target_link_libraries(generate_nfa ${Boost_LIBRARIES})

install (TARGETS generate_nfa DESTINATION bin)