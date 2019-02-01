#
# Copyright (C) 2019 Tomasz Walczyk
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.
#
###########################################################

cmake_minimum_required(VERSION 3.12)
project(windows-mkpasswd VERSION 0.0.1.0)

if (NOT WIN32)
  message(FATAL_ERROR "Windows is required to compile this project!")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(test)
