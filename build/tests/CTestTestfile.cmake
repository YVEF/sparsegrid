# CMake generated Testfile for 
# Source directory: /home/iaroslav/src/sparsegrid/tests
# Build directory: /home/iaroslav/src/sparsegrid/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[test_board]=] "/home/iaroslav/src/sparsegrid/build/tests/sparsegridtest" "--log_level=all" "--report_level=short" "--show-progress=on" "--run_test=board_test_suite")
set_tests_properties([=[test_board]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;25;add_test;/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;0;")
add_test([=[test_board_state]=] "/home/iaroslav/src/sparsegrid/build/tests/sparsegridtest" "--log_level=all" "--report_level=short" "--show-progress=on" "--run_test=board_state_test_suite")
set_tests_properties([=[test_board_state]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;26;add_test;/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;0;")
add_test([=[test_undo]=] "/home/iaroslav/src/sparsegrid/build/tests/sparsegridtest" "--log_level=all" "--report_level=short" "--show-progress=on" "--run_test=undo_test_suite")
set_tests_properties([=[test_undo]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;27;add_test;/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;0;")
add_test([=[test_hashing]=] "/home/iaroslav/src/sparsegrid/build/tests/sparsegridtest" "--log_level=all" "--report_level=short" "--show-progress=on" "--run_test=hashing_test_suite")
set_tests_properties([=[test_hashing]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;28;add_test;/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;0;")
add_test([=[test_fen]=] "/home/iaroslav/src/sparsegrid/build/tests/sparsegridtest" "--log_level=all" "--report_level=short" "--show-progress=on" "--run_test=fen_test_suite")
set_tests_properties([=[test_fen]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;29;add_test;/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;0;")
add_test([=[test_search]=] "/home/iaroslav/src/sparsegrid/build/tests/sparsegridtest" "--log_level=all" "--report_level=short" "--show-progress=on" "--run_test=mtdsearch_test_suite")
set_tests_properties([=[test_search]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;30;add_test;/home/iaroslav/src/sparsegrid/tests/CMakeLists.txt;0;")
