cmake_minimum_required(VERSION 3.13)
project(cse498-segfault-survivors)

# Configuration options
set(SDL_VERSION "2" CACHE STRING "SDL version 2 or 3")
set(BUILD_OUTPUT "html" CACHE STRING "Output format")
set(BUILD_TESTS OFF CACHE BOOL "Build tests")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: This section might need work. I am not sure what to anticpate just yet.
# Source files
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Header directories
set(HEADER_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Agents
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/Worlds
)

if(BUILD_TESTS)
    # Fetch Catch2 v3 from GitHub
    # We can change this if we would like to use the version in the third-party directory
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.12.0
    )
    FetchContent_MakeAvailable(Catch2)

    # Exclude main source files from library
    list(FILTER SOURCES EXCLUDE REGEX "(_main|^main)\\.cpp$")

    # Create INTERFACE library if header-only, regular if sources exist
    list(LENGTH SOURCES SOURCES_COUNT)
    if(SOURCES_COUNT EQUAL 0)
        add_library(project_lib INTERFACE)
        target_include_directories(project_lib INTERFACE ${HEADER_DIRS})
    else()
        add_library(project_lib ${SOURCES})
        target_include_directories(project_lib PUBLIC ${HEADER_DIRS})
    endif()

    # Create test executable
    file(GLOB TEST_SOURCES "../tests/*.cpp")
    list(LENGTH TEST_SOURCES TEST_SOURCES_COUNT)
    if(TEST_SOURCES_COUNT EQUAL 0)
        message(FATAL_ERROR "No test files found in tests/ directory")
    endif()
    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE project_lib Catch2::Catch2WithMain)

    # Emscripten compatibility: disable POSIX signals
    target_compile_definitions(tests PRIVATE CATCH_CONFIG_NO_POSIX_SIGNALS)

    # Output as a js file for Node.js execution
    set_target_properties(tests PROPERTIES SUFFIX ".js")
    target_link_options(tests PRIVATE
        -sNODERAWFS=1
        -sALLOW_MEMORY_GROWTH=1
    )
else()
    # Emscripten build
    # Determine output suffix
    if(BUILD_OUTPUT STREQUAL "html")
        set(OUTPUT_SUFFIX ".html")
    else()
        set(OUTPUT_SUFFIX ".js")
    endif()

    # Create executable
    add_executable(index ${SOURCES})
    set_target_properties(index PROPERTIES SUFFIX ${OUTPUT_SUFFIX})
    target_include_directories(index PRIVATE ${HEADER_DIRS})

    # Emscripten compile options
    target_compile_options(index PRIVATE
        -O2
        -Wall
        --use-port=sdl${SDL_VERSION}
    )

    # Emscripten link options
    target_link_options(index PRIVATE
        --use-port=sdl${SDL_VERSION}
        -sALLOW_MEMORY_GROWTH=1
        -sMAXIMUM_MEMORY=1gb
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
        --emrun
    )
endif()
