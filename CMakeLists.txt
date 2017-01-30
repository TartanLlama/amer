cmake_minimum_required (VERSION 2.6)

project (Amer)

include(ext/ext.cmake)

include_directories("include" "ext/koura" "ext/cxxopts/include" "ext/cpptoml/include" "ext/koura" "ext/onion/src/bindings/cpp" "ext/onion/src")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -static-libstdc++")

add_executable(amer
        src/server.cpp
        src/main.cpp)

set (CPPNETLIB_LIBS libcppnetlib-client-connections libcppnetlib-server-parsers)

target_link_libraries(amer stdc++fs libcmark_static pthread onion onioncpp)
