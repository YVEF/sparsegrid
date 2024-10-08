cmake_minimum_required(VERSION 3.29.6)
if (NOT EXISTS $ENV{VCPKG_ROOT})
	message(FATAL_ERROR "$VCPKG_ROOT is not defined")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(Boost_NO_WARN_NEW_VERSIONS 1)
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(VCPKG_LOAD_VCVARS_ENV ON)
project(sparsegrid VERSION 0.0.2)

set(PROJECT_LIB_NAME ${PROJECT_NAME}core)
set(PROJECT_TEST_NAME ${PROJECT_NAME}test)
set(PROJECT_BENCH_NAME ${PROJECT_NAME}perft)
set(PROJECT_MICRO_BENCH_NAME ${PROJECT_NAME}_micro_bench)
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
add_definitions(-DDEV_ENV)
add_definitions(-DTEST_ENV)

message(STATUS "!!!!!11-------!!!   " ${CMAKE_CXX_COMPILER_ID})

add_compile_options(-Wall -Wpedantic -Wextra -mavx2)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Debug build type")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(-g -O0
             -fsanitize=address
             -fno-omit-frame-pointer
             -fsanitize-address-destructor=global
             -fsanitize-address-globals-dead-stripping
             -fsanitize-address-outline-instrumentation
             -fsanitize-address-use-after-scope
             -fsanitize-link-c++-runtime
             -fsanitize-stats
             )
         add_link_options(-fuse-ld=lld -fsanitize=address)
    else ()
        add_compile_options(-g -O0
#                -fsanitize=address
        )
#        add_link_options(-fsanitize=address -static-libasan)
    endif ()
    add_definitions(-DDEBUG)
else()
    message(STATUS "Release build type")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(-O3
             -ffast-math
             -flto=full
             -finline-functions
             -funsafe-math-optimizations
             )
         add_link_options(-flto=full)
    else ()
        add_compile_options(-O3
            -finline-functions
            -funsafe-math-optimizations
            -flto
        )
    endif ()
    add_definitions(-DNDEBUG)
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(perft)


