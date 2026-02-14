cmake_minimum_required(VERSION 3.13)
project(cse498-segfault-survivors)

# Build tests by default now since we are focusing on them
set(BUILD_TESTS ON CACHE BOOL "Build tests")
set(CMAKE_CXX_STANDARD 23)

# Source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Header directories
set(HEADER_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Agents
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/tools
        ${CMAKE_CURRENT_SOURCE_DIR}/Worlds
)

# --- TEST BUILD (Node.js) ---
if(BUILD_TESTS)
    # Define dummy library
    add_library(project_lib INTERFACE)
    target_include_directories(project_lib INTERFACE ${HEADER_DIRS})

    add_executable(tests "tests/WebTextboxTests.cpp" "Interfaces/WebTextbox.cpp")
    target_link_libraries(tests PRIVATE project_lib)

    # CRITICAL: Build as .js for Node.js
    set_target_properties(tests PROPERTIES SUFFIX ".js")

    # CRITICAL: Link options for Node.js environment
    target_link_options(tests PRIVATE
            -sALLOW_MEMORY_GROWTH=1
            -sEXIT_RUNTIME=1
            -sNODERAWFS=1      # Allows direct file system access (fast logging)
            -sENVIRONMENT=node # Explicitly target Node.js
            -lembind
    )
endif()