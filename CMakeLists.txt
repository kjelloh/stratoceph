cmake_minimum_required(VERSION 3.15)
project(stratoceph CXX)




add_library(stratoceph src/stratoceph.cpp)
target_include_directories(stratoceph PUBLIC include)



set_target_properties(stratoceph PROPERTIES PUBLIC_HEADER "include/stratoceph.h")
install(TARGETS stratoceph)
