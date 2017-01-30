#
# add cmark (stolen from Standardese)
#
message(STATUS ${CMARK_LIBRARY})

find_library(CMARK_LIBRARY "cmark" "/usr/lib" "/usr/local/lib")
find_path(CMARK_INCLUDE_DIR "cmark.h" "/usr/include" "/usr/local/include")

if((NOT CMARK_LIBRARY) OR (NOT CMARK_INCLUDE_DIR))
    message("Unable to find cmark, cloning...")
    execute_process(COMMAND git submodule update --init -- ext/cmark
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # add and exclude targets
    add_subdirectory(ext/cmark ${CMAKE_CURRENT_BINARY_DIR}/cmark)

    # fixup target properties
    target_include_directories(libcmark_static PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cmark/src>
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/cmark/src>)
                               target_compile_definitions(libcmark_static PUBLIC CMARK_STATIC_DEFINE)

    # disable some warnings under MSVC, they're very noisy
    if(MSVC)
        target_compile_options(libcmark_static PRIVATE /wd4204 /wd4267 /wd4204 /wd4221 /wd4244 /wd4232)
    endif()
else()
    add_library(libcmark_static INTERFACE)
    target_include_directories(libcmark_static INTERFACE ${CMARK_INCLUDE_DIR})
    target_link_libraries(libcmark_static INTERFACE ${CMARK_LIBRARY})

    # install fake target
    install(TARGETS libcmark_static EXPORT standardese DESTINATION ${lib_dir})
endif()


#
# add onion
#

# make sure the module path is set correctly
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/ext/onion/CMakeModules")
add_subdirectory(ext/onion ${CMAKE_CURRENT_BINARY_DIR}/onion)
