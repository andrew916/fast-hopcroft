add_executable(minimize main.cpp)

target_link_libraries(minimize ${Boost_LIBRARIES})

install (TARGETS minimize DESTINATION bin)