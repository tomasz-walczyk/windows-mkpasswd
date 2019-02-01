#
# Copyright (C) 2019 Tomasz Walczyk
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.
#
###########################################################

set(GOOGLE_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/GoogleTest)
set(GOOGLE_TEST_INSTALL_DIR ${GOOGLE_TEST_DIR}/install)

include(ExternalProject)
ExternalProject_Add(GoogleTest
  EXCLUDE_FROM_ALL ON
  PREFIX ${GOOGLE_TEST_DIR}
  INSTALL_DIR ${GOOGLE_TEST_INSTALL_DIR}
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG master
  CMAKE_CACHE_ARGS
    -DBUILD_GMOCK:BOOL=ON
    -DINSTALL_GTEST:BOOL=ON
    -Dgtest_force_shared_crt:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>/$<CONFIG>
)

set(GOOGLE_TEST_INCLUDE_DIRECTORIES ${GOOGLE_TEST_INSTALL_DIR}/$<CONFIG>/include)
foreach(LIBRARY gtest gmock)
  add_library(${LIBRARY} STATIC IMPORTED)
  set_target_properties(${LIBRARY} PROPERTIES IMPORTED_LOCATION_DEBUG ${GOOGLE_TEST_INSTALL_DIR}/Debug/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${LIBRARY}d${CMAKE_STATIC_LIBRARY_SUFFIX})
  set_target_properties(${LIBRARY} PROPERTIES IMPORTED_LOCATION_RELEASE ${GOOGLE_TEST_INSTALL_DIR}/Release/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX})
  add_dependencies(${LIBRARY} GoogleTest)
endforeach()

add_executable(windows-mkpasswd-test source/main.cpp source/windows-mkpasswd.cpp)
target_link_libraries(windows-mkpasswd-test PRIVATE gtest gmock windows-mkpasswd-lib)
target_include_directories(windows-mkpasswd-test PRIVATE ${GOOGLE_TEST_INCLUDE_DIRECTORIES})
target_compile_definitions(windows-mkpasswd-test PRIVATE -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
