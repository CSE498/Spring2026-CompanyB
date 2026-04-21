cmake_minimum_required(VERSION 3.13)
project(cse498-segfault-survivors)

# Configuration options
set(BUILD_TESTS OFF CACHE BOOL "Build native tests")
set(BUILD_WASM_TESTS OFF CACHE BOOL "Build Emscripten specific wasm tests")
set(NO_QT OFF CACHE BOOL "Build without Qt and exclude Interfaces/gui sources")
set(TARGET_MAIN "simple_main.cpp" CACHE STRING "Entry point file (must end in _main.cpp)")
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
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/webui
    ${CMAKE_CURRENT_SOURCE_DIR}/worlds
)

include(FetchContent)
FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG 0.12.0
)
# Deactivate the JSON library tests
# https://github.com/nlohmann/json?tab=readme-ov-file#embedded
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(range-v3 nlohmann_json)


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
  list(FILTER SOURCES EXCLUDE REGEX "(_main|/main)\\.cpp$")
  list(FILTER SOURCES EXCLUDE REGEX "web_entry.cpp$")

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
  target_link_libraries(project_lib PUBLIC nlohmann_json::nlohmann_json)
  target_include_directories(project_lib SYSTEM PUBLIC
    $<TARGET_PROPERTY:range-v3,INTERFACE_INCLUDE_DIRECTORIES>
)

  # Emscripten compatibility: disable POSIX signals
  target_compile_definitions(tests PRIVATE CATCH_CONFIG_NO_POSIX_SIGNALS)

  # Install npm modules for jsdom and canvas in the build directory
  execute_process(
      COMMAND npm install jsdom canvas
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

  # Output as a js file for Node.js execution
  set_target_properties(tests PROPERTIES SUFFIX ".js")
  message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
  target_link_options(tests PRIVATE
        -sNODERAWFS=1
        -sALLOW_MEMORY_GROWTH=1
        -lembind
        # -fexceptions
        -sNO_DISABLE_EXCEPTION_CATCHING
        # Link JS libraries for Node
        --pre-js "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/emscripten-test-libs/pre.js"
    )

elseif(BUILD_TESTS)
  # Exclude Emscripten only sources
  list(FILTER SOURCES EXCLUDE REGEX "(_main|/main)\\.cpp$")
  list(FILTER SOURCES EXCLUDE REGEX ".*/tools/webui/.*")

  if(NO_QT)
    # Skip Qt entirely and exclude GUI sources
    list(FILTER SOURCES EXCLUDE REGEX ".*/Interfaces/gui/.*")
  else()
    # Find Qt6 for native build
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
    set(CMAKE_AUTOMOC ON) # Necessary for QT macro searches
  endif()

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
    if(NOT NO_QT)
      target_link_libraries(project_lib PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
    endif()
  endif()

  # Native tests from tests/ (excluding tests/wasm/)
  file(GLOB_RECURSE TEST_SOURCES "../tests/*.cpp")
  list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/wasm/.*")
  if(NO_QT)
    # Exclude tests for Qt GUI components
    list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/tests/tools/.*")
  endif()
  list(LENGTH TEST_SOURCES TEST_SOURCES_COUNT)
  if(TEST_SOURCES_COUNT EQUAL 0)
    message(FATAL_ERROR "No test files found in tests/ directory")
  endif()
  add_executable(tests ${TEST_SOURCES})
  target_link_libraries(tests PRIVATE project_lib Catch2::Catch2WithMain)
  target_link_libraries(project_lib PUBLIC range-v3 nlohmann_json::nlohmann_json)

elseif(DEFINED EMSCRIPTEN)
  # Exclude Qt GUI files (Qt can't be compiled by emcc)
  list(FILTER SOURCES EXCLUDE REGEX ".*/Interfaces/gui/.*")

  # Exclude all entry points then add back the selected one
  list(FILTER SOURCES EXCLUDE REGEX ".*_main\\.cpp$")
  list(FILTER SOURCES EXCLUDE REGEX ".*/webui/web_entry\\.cpp$")
  # Default to web_main.cpp for Emscripten builds if no target was specified
  if(TARGET_MAIN STREQUAL "simple_main.cpp")
    set(TARGET_MAIN "web_main.cpp")
  endif()
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_MAIN}")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_MAIN}")
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../demo/${TARGET_MAIN}")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../demo/${TARGET_MAIN}")
  else()
    message(FATAL_ERROR "TARGET_MAIN '${TARGET_MAIN}' not found in source/ or demo/")
  endif()

  add_executable(index ${SOURCES})
  set_target_properties(index PROPERTIES SUFFIX ".js")
  target_include_directories(index PRIVATE ${HEADER_DIRS})
  target_link_libraries(index PRIVATE range-v3 nlohmann_json::nlohmann_json)

  # Emscripten compile options
  target_compile_options(index PRIVATE
        -O2
        -Wall
        # -g
        # -fno-inline
    )

  # Emscripten link options
  target_link_options(index PRIVATE
        # -g
        -sALLOW_MEMORY_GROWTH=1
        -sMAXIMUM_MEMORY=1gb
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
        -lembind
        # Embed grid files into the WASM virtual filesystem so C++ ifstream can open them.
        # SHELL: prefix prevents CMake from splitting the @ separator into two arguments.
        "SHELL:--preload-file /app/assets/grids/TrafficWorld_Web.grid@assets/grids/TrafficWorld_Web.grid"
    )

  # Copy custom HTML into build output
  configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/webui/index.html
        ${CMAKE_BINARY_DIR}/index.html
        COPYONLY
    )

  configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/webui/menu.html
        ${CMAKE_BINARY_DIR}/menu.html
        COPYONLY
    )

  configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/webui/style.css
        ${CMAKE_BINARY_DIR}/style.css
        COPYONLY
    )

  set(ASSETS_SOURCE_DIR "/app/assets")
  set(ASSETS_BUILD_DIR "${CMAKE_BINARY_DIR}/assets")
  add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${ASSETS_SOURCE_DIR}"
            "${ASSETS_BUILD_DIR}"
        COMMENT "Copying assets directory"
    )
  add_dependencies(index copy_assets)


else()
  # Native build
  list(FILTER SOURCES EXCLUDE REGEX ".*/tools/webui/.*")
  # Exclude all entry points then add back the selected one
  list(FILTER SOURCES EXCLUDE REGEX ".*_main\\.cpp$")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_MAIN}")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_MAIN}")
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../demo/${TARGET_MAIN}")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../demo/${TARGET_MAIN}")
  else()
    message(FATAL_ERROR "TARGET_MAIN '${TARGET_MAIN}' not found in source/ or demo/")
  endif()

  if(NO_QT)
    # Skip Qt entirely and exclude GUI sources
    list(FILTER SOURCES EXCLUDE REGEX ".*/Interfaces/gui/.*")
  else()
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
    set(CMAKE_AUTOMOC ON)
  endif()

  add_executable(app ${SOURCES})
  target_link_libraries(app PRIVATE range-v3 nlohmann_json::nlohmann_json)
  target_include_directories(app PRIVATE ${HEADER_DIRS})
  if(NOT NO_QT)
    target_link_libraries(app PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
  endif()

endif()

message(STATUS "Build type:  ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX flags:   ${CMAKE_CXX_FLAGS}")
message(STATUS "NO_QT:       ${NO_QT}")
message(STATUS "TARGET_MAIN: ${TARGET_MAIN}")
