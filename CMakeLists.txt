cmake_minimum_required(VERSION 3.15)
project(stratoceph CXX)

set(CMAKE_CXX_STANDARD 23)

# find_package(glfw3)
# find_package(OpenCASCADE REQUIRED)
# find_package(imgui REQUIRED)
# find_package(litehtml)
# find_package(GTest REQUIRED)
# find_package(immer REQUIRED)
find_package(pugixml REQUIRED)
# find_package(spdlog REQUIRED)

add_library(stratoceph src/stratoceph.cpp)
target_include_directories(stratoceph PUBLIC include)

# target_link_libraries(stratoceph glfw)
# target_link_libraries(stratoceph opencascade::opencascade)
# target_link_libraries(stratoceph imgui::imgui)
# target_link_libraries(stratoceph litehtml)
# target_link_libraries(stratoceph gtest::gtest)
# target_link_libraries(stratoceph immer::immer)
target_link_libraries(stratoceph pugixml::pugixml)
# target_link_libraries(stratoceph spdlog::spdlog)

set_target_properties(stratoceph PROPERTIES
    PUBLIC_HEADER 
    "include/stratoceph/stratoceph.h")

install(TARGETS stratoceph
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stratoceph)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
