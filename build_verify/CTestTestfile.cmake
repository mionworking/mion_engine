# CMake generated Testfile for 
# Source directory: /home/danten/Documents/G_v2/mion_engine_cpp
# Build directory: /home/danten/Documents/G_v2/mion_engine_cpp/build_verify
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[mion_tests_v2]=] "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/mion_tests_v2")
set_tests_properties([=[mion_tests_v2]=] PROPERTIES  LABELS "official" _BACKTRACE_TRIPLES "/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;140;add_test;/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;0;")
add_test([=[mion_tests_v2_core]=] "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/mion_tests_v2_core")
set_tests_properties([=[mion_tests_v2_core]=] PROPERTIES  LABELS "official" _BACKTRACE_TRIPLES "/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;146;add_test;/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;0;")
add_test([=[mion_tests_v2_scenes]=] "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/mion_tests_v2_scenes")
set_tests_properties([=[mion_tests_v2_scenes]=] PROPERTIES  LABELS "official" _BACKTRACE_TRIPLES "/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;152;add_test;/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;0;")
add_test([=[mion_tests_v2_input]=] "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/mion_tests_v2_input")
set_tests_properties([=[mion_tests_v2_input]=] PROPERTIES  LABELS "official" _BACKTRACE_TRIPLES "/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;158;add_test;/home/danten/Documents/G_v2/mion_engine_cpp/CMakeLists.txt;0;")
subdirs("_deps/sdl3-build")
