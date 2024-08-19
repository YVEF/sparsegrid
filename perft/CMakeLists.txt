include_directories(
        ${CMAKE_SOURCE_DIR}/src)

add_executable(${PROJECT_BENCH_NAME}
        main.cpp
        runner.cpp)

target_link_libraries(${PROJECT_BENCH_NAME} PRIVATE ${PROJECT_LIB_NAME})
