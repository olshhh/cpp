cmake_minimum_required(VERSION 3.28)

project(example_06_05 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost 1.85 REQUIRED COMPONENTS filesystem system)

add_library(version_a SHARED lib_version_a.cpp)
add_library(version_b SHARED lib_version_b.cpp)

add_executable(main_app 06_05.cpp)

target_compile_options(version_a PRIVATE -Wall -Wextra -Wpedantic -O0)
target_compile_options(version_b PRIVATE -Wall -Wextra -Wpedantic -O0)
target_compile_options(main_app PRIVATE -Wall -Wextra -Wpedantic -O0)

target_link_libraries(main_app PRIVATE Boost::filesystem Boost::system)

set_target_properties(
    version_a
    version_b
    main_app
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)