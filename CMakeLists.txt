cmake_minimum_required(VERSION 3.15)
project(stratoceph CXX)

add_library(stratoceph src/stratoceph.cpp)
target_include_directories(stratoceph PUBLIC include)

set_target_properties(stratoceph PROPERTIES PUBLIC_HEADER "include/stratoceph/stratoceph.h")

# It seems to have header files in target preserve directory structure we have to 
# tell cmake a properietary 'DESTINATION'?
install(TARGETS stratoceph
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stratoceph)
