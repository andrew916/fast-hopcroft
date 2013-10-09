add_executable(determinize main.cpp)
target_link_libraries(determinize ${Boost_LIBRARIES})
install (TARGETS determinize DESTINATION bin)
