find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(
	${Boost_INCLUDE_DIRS} 
	${CMAKE_SOURCE_DIR}/src)



set(TESTS_SRC
    test_utils.cpp
    test_board.cpp
    test_board_state.cpp
    test_undo.cpp
    test_hashing.cpp
    test_mtdsearch.cpp
    test_fen.cpp
	test_polyglot.cpp
)


add_executable(${PROJECT_TEST_NAME} main.cpp ${TESTS_SRC})
target_link_libraries(${PROJECT_TEST_NAME} PRIVATE ${PROJECT_LIB_NAME} Boost::unit_test_framework)
# target_compile_definitions(${PROJECT_TEST_NAME} PRIVATE "OB_POLY_BIN=\"${OB_POLY_BIN}\"")

set(BOOST_TEST_STD_ARGS --log_level=all --report_level=short --show-progress=on)
add_test(NAME test_board COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=board_test_suite)
add_test(NAME test_board_state COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=board_state_test_suite)
add_test(NAME test_undo COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=undo_test_suite)
add_test(NAME test_hashing COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=hashing_test_suite)
add_test(NAME test_fen COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=fen_test_suite)
add_test(NAME test_search COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=mtdsearch_test_suite)
# add_test(NAME test_polyglot COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=polyglot_test_suite)
# add_test(NAME test_opening_book COMMAND ${PROJECT_TEST_NAME} ${BOOST_TEST_STD_ARGS} --run_test=opening_book_test_suite)
