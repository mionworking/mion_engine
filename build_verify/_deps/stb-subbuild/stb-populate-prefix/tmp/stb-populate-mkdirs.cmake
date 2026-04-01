# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-src"
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-build"
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix"
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix/tmp"
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp"
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix/src"
  "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/danten/Documents/G_v2/mion_engine_cpp/build_verify/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
