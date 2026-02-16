cmake_minimum_required(VERSION 3.13)
project(cse498-segfault-survivors)

set(BUILD_TESTS OFF CACHE BOOL "Build tests")
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(HEADER_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Agents
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/tools
        ${CMAKE_CURRENT_SOURCE_DIR}/Worlds
)

if(BUILD_TESTS)
    # --- TEST BUILD (Node.js) ---
    add_library(project_lib INTERFACE)
    target_include_directories(project_lib INTERFACE ${HEADER_DIRS})

    add_executable(tests "tests/WebTextboxTests.cpp" "Interfaces/WebTextbox.cpp")
    target_link_libraries(tests PRIVATE project_lib)

    set_target_properties(tests PROPERTIES SUFFIX ".js")
    target_link_options(tests PRIVATE
        -sNODERAWFS=1
        -sALLOW_MEMORY_GROWTH=1
        -lembind
    )
else()
    # --- VISUAL DEMO BUILD (HTML) ---

    # Exclude tests from the visual build so they don't crash the browser
    list(FILTER SOURCES EXCLUDE REGEX "tests/.*")

    add_executable(index ${SOURCES})
    set_target_properties(index PROPERTIES SUFFIX ".html")
    target_include_directories(index PRIVATE ${HEADER_DIRS})

    target_compile_options(index PRIVATE -O2 -Wall)

    target_link_options(index PRIVATE
            -sALLOW_MEMORY_GROWTH=1
            -sMAXIMUM_MEMORY=1gb
            -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
            --emrun
            -lembind
    )
endif()
