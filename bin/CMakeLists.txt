#
# Copyright (C) 2019 Tomasz Walczyk
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.
#
###########################################################

add_executable(windows-mkpasswd-bin source/main.cpp)
set_target_properties(windows-mkpasswd-bin PROPERTIES OUTPUT_NAME windows-mkpasswd)
target_link_libraries(windows-mkpasswd-bin PRIVATE windows-mkpasswd-lib)
install(TARGETS windows-mkpasswd-bin DESTINATION bin)
