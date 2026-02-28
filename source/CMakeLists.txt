cmake_minimum_required(VERSION 3.13)
project(cse498-segfault-survivors)

# Configuration options
set(BUILD_TESTS OFF CACHE BOOL "Build native tests")
set(BUILD_WASM_TESTS OFF CACHE BOOL "Build Emscripten specific wasm tests")
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
    ${CMAKE_CURRENT_SOURCE_DIR}/agents
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/worlds
)

if(BUILD_WASM_TESTS)
    # Exclude Qt GUI files (Qt can't be compiled by emcc)
    list(FILTER SOURCES EXCLUDE REGEX ".*/Interfaces/gui/.*")

    # Fetch Catch2 v3 from GitHub
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.12.0
    )
    FetchContent_MakeAvailable(Catch2)

    # Exclude main source files from library
    list(FILTER SOURCES EXCLUDE REGEX "(_main|^main)\\.cpp$")

    # Create INTERFACE library if header only, regular if sources exist
    list(LENGTH SOURCES SOURCES_COUNT)
    if(SOURCES_COUNT EQUAL 0)
        add_library(project_lib INTERFACE)
        target_include_directories(project_lib INTERFACE ${HEADER_DIRS})
    else()
        add_library(project_lib ${SOURCES})
        target_include_directories(project_lib PUBLIC ${HEADER_DIRS})
    endif()

    # Emscripten specific tests from tests/wasm/
    file(GLOB_RECURSE TEST_SOURCES "../tests/wasm/*.cpp")
    list(LENGTH TEST_SOURCES TEST_SOURCES_COUNT)
    if(TEST_SOURCES_COUNT EQUAL 0)
        message(FATAL_ERROR "No test files found in tests/wasm/ directory")
    endif()
    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE project_lib Catch2::Catch2WithMain)

    # Emscripten compatibility: disable POSIX signals
    target_compile_definitions(tests PRIVATE CATCH_CONFIG_NO_POSIX_SIGNALS)

    # Copy over the files needed by jsdom
    file(COPY "/src/node_modules/jsdom/lib/jsdom/"
        DESTINATION "/app/build/")
    file(COPY "/src/node_modules/jsdom/lib/jsdom/living/xhr/xhr-sync-worker.js"
     DESTINATION "${CMAKE_BINARY_DIR}/")

    # Output as a js file for Node.js execution
    set_target_properties(tests PROPERTIES SUFFIX ".js")
    message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
    target_link_options(tests PRIVATE
        -sNODERAWFS=1
        -sALLOW_MEMORY_GROWTH=1
        -lembind
        -sNO_DISABLE_EXCEPTION_CATCHING
        # --pre-js "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/jsdom/jsdom.min.js"
        --pre-js /app/jsdom.min.js
    )

elseif(BUILD_TESTS)
    # Exclude Emscripten only sources
    list(FILTER SOURCES EXCLUDE REGEX "(_main|^main)\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX ".*/tools/webui/.*")

    # Find Qt6 for native build
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
    set(CMAKE_AUTOMOC ON) # Necessary for QT macro searches

    # Fetch Catch2 v3 from GitHub
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.12.0
    )
    FetchContent_MakeAvailable(Catch2)

    # Create INTERFACE library if header only, regular if sources exist
    list(LENGTH SOURCES SOURCES_COUNT)
    if(SOURCES_COUNT EQUAL 0)
        add_library(project_lib INTERFACE)
        target_include_directories(project_lib INTERFACE ${HEADER_DIRS})
    else()
        add_library(project_lib ${SOURCES})
        target_include_directories(project_lib PUBLIC ${HEADER_DIRS})
        target_link_libraries(project_lib PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
    endif()

    # Native tests from tests/ (excluding tests/wasm/)
    file(GLOB_RECURSE TEST_SOURCES "../tests/*.cpp")
    list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/wasm/.*")
    list(LENGTH TEST_SOURCES TEST_SOURCES_COUNT)
    if(TEST_SOURCES_COUNT EQUAL 0)
        message(FATAL_ERROR "No test files found in tests/ directory")
    endif()
    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE project_lib Catch2::Catch2WithMain)
    target_compile_options(tests PRIVATE -O2 -g -Wall -Wextra -Wpedantic)

elseif(DEFINED EMSCRIPTEN)
    # Exclude Qt GUI files (Qt can't be compiled by emcc)
    list(FILTER SOURCES EXCLUDE REGEX ".*/Interfaces/gui/.*")

    add_executable(index ${SOURCES})
    set_target_properties(index PROPERTIES SUFFIX ".html")
    target_include_directories(index PRIVATE ${HEADER_DIRS})

    # Emscripten compile options
    target_compile_options(index PRIVATE
        -O2
        -Wall
    )

    # Emscripten link options
    target_link_options(index PRIVATE
        -sALLOW_MEMORY_GROWTH=1
        -sMAXIMUM_MEMORY=1gb
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
        --emrun
        -lembind
    )

else()
    # Native build: include Qt GUI files
    list(FILTER SOURCES EXCLUDE REGEX ".*/tools/webui/.*")

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
    set(CMAKE_AUTOMOC ON)

    add_executable(app ${SOURCES})
    target_include_directories(app PRIVATE ${HEADER_DIRS})
    target_link_libraries(app PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
    target_compile_options(app PRIVATE
        -Wall -Wextra -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual -pedantic
    )

endif()
