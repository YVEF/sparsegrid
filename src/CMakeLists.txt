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
        search/book.cpp
#       search/tracer.cpp
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
        core/CallerThreadExecutor.cpp
        core/ThreadPoolExecutor.cpp
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

set(INTEROP_PROJ sg_trainer_interop)
add_library(${PROJECT_LIB_NAME} STATIC  ${PROJ_SOURCES})
add_library(${INTEROP_PROJ} SHARED  ${PROJ_SOURCES} interop.cpp)



find_package(Python3 REQUIRED COMPONENTS Interpreter Development NumPy)
find_package(Boost COMPONENTS REQUIRED python numpy)
find_package(Eigen3 CONFIG REQUIRED)
#find_package(Boost COMPONENTS REQUIRED iostreams)
find_path(GZIP_HPP_INCLUDE_DIRS "gzip/compress.hpp")
find_package(ZLIB)

include_directories(${EIGEN3_INCLUDE_DIR} ${GZIP_HPP_INCLUDE_DIRS}) # todo: fix
#target_include_directories(${PROJECT_LIB_NAME} PUBLIC ${EIGEN3_INCLUDE_DIR})
target_link_libraries(${PROJECT_LIB_NAME} PUBLIC Eigen3::Eigen ZLIB::ZLIB)

set(PROJECT_NAME_EXE ${PROJECT_NAME})
add_executable(${PROJECT_NAME_EXE} main.cpp)
target_link_libraries(${PROJECT_NAME_EXE} PUBLIC ${PROJECT_LIB_NAME})



target_link_libraries(${INTEROP_PROJ} PUBLIC
        Boost::python
        Boost::numpy
        Python3::Python
        Python3::NumPy
        ZLIB::ZLIB
)

target_include_directories(${INTEROP_PROJ} PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIR}
        ${Python3_NumPy_INCLUDE_DIRS})

set_target_properties(${INTEROP_PROJ} PROPERTIES PREFIX "")
add_custom_command(TARGET ${INTEROP_PROJ} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${INTEROP_PROJ}> ${CMAKE_SOURCE_DIR}/sgtrain/${INTEROP_PROJ}.so
)

