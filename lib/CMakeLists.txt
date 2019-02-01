#
# Copyright (C) 2019 Tomasz Walczyk
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.
#
###########################################################

add_library(windows-mkpasswd-lib STATIC include/windows-mkpasswd/windows-mkpasswd.hpp source/windows-mkpasswd.cpp)
set_target_properties(windows-mkpasswd-lib PROPERTIES OUTPUT_NAME windows-mkpasswd)
target_include_directories(windows-mkpasswd-lib PUBLIC include)
install(TARGETS windows-mkpasswd-lib DESTINATION $<CONFIG>/lib)
install(DIRECTORY include DESTINATION $<CONFIG>)