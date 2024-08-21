set(BOARD_SRC 
    board/board.cpp
    board/board_state.cpp
    board/move.cpp
    board/pieces.cpp
    board/movegen.cpp
)


set(EVAL_SRC 
    eval/evaluator.cpp
)

set(SEARCH_SRC 
#     search/work_context.cpp
     search/book.cpp
#     search/tracer.cpp
    search/tt.cpp
    search/tm.cpp
    search/mtdsearch.cpp
    search/tm.cpp
)

set(DBG_UTILS_SRC 
        dbg/sg_log.cpp
        dbg/debugger.cpp
)

set(CORE_SRC 
        core/defs.cpp
        core/caller_thread_executor.cpp
        # core/sg_intrin.cpp
        # core/thread_pool_executor.cpp
)

set(UCI_SRC 
        uci/fen.cpp
        uci/protocol.cpp
)

set(ENGINE_SRC
        engine.cpp
)

 set(ADAPTERS_SRC
        adapters/polyglot.cpp
        adapters/polyglot_hashkeys.cpp
 )

set(COMMON_SRC
    common/stat.cpp
)

## === DEFS
if("${CMAKE_BUILD_TYPE}" STREQUAL  "Debug")
    add_definitions(-DENABLE_SG_ASSERT)
    add_definitions(-DENABLE_SG_ASSERT_DBG)
endif()
## ===

#find_package(PythonLibs 2.7 REQUIRED)
#find_package (Python3 COMPONENTS Interpreter Development NumPy)
#find_package(Boost COMPONENTS python REQUIRED)

set(PROJ_SOURCES ${BOARD_SRC}
        ${EVAL_SRC}
        ${SEARCH_SRC}
        ${DBG_UTILS_SRC}
        ${CORE_SRC}
        ${UCI_SRC}
        ${ENGINE_SRC}
        ${ADAPTERS_SRC}
        ${COMMON_SRC}
)

add_library(${PROJECT_LIB_NAME} STATIC  ${PROJ_SOURCES})
add_library(${PROJECT_LIB_NAME}shared SHARED  ${PROJ_SOURCES} interop.cpp interop2.cpp)


find_package(Eigen3 CONFIG REQUIRED)
target_include_directories(${PROJECT_LIB_NAME} PUBLIC ${EIGEN3_INCLUDE_DIR})

set(PROJECT_NAME_EXE ${PROJECT_NAME})
add_executable(${PROJECT_NAME_EXE} main.cpp)
target_link_libraries(${PROJECT_NAME_EXE} PUBLIC ${PROJECT_LIB_NAME} Eigen3::Eigen ${Boost_LIBRARIES})
#target_include_directories(${PROJECT_NAME_EXE} PRIVATE ${Python3_INCLUDE_DIRS})
install(TARGETS ${PROJECT_NAME_EXE} DESTINATION lib)
