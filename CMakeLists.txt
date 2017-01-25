cmake_minimum_required (VERSION 2.6)

project (Amer)

include(ext/ext.cmake)

include_directories("include" "ext/koura/koura" "ext/cxxopts/include" "ext/cpptoml/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_executable(amer
        src/main.cpp)

target_link_libraries(amer stdc++fs libcmark_static)
