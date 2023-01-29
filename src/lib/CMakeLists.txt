# bluecheese
# Copyright (C) 2022-2023 Chris January
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program.  If not, see
# <https://www.gnu.org/licenses/>.

add_library(chessboard SHARED
  include/chessboard_global.h
  include/chessboard.h
  boardaddress.cpp
  boardaddress_p.h
  boardstate.cpp
  bleboardfactory.h
  bleboardfactory.cpp
  bleconnection.h
  bleconnection.cpp
  chessupboard.cpp
  chessupboard.h
  connection.h
  connectionfactory.h
  connectionfactory.cpp
  connectionmanager.cpp
  connectionmanager_p.h
  discovery.cpp
  remoteboard_p.h
  remoteboard.cpp
)

target_include_directories(chessboard PUBLIC include)

target_link_libraries(chessboard PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Bluetooth
)

target_compile_definitions(chessboard PRIVATE LIBCHESSBOARD_LIBRARY)

set_target_properties(chessboard PROPERTIES OUTPUT_NAME bluecheese)
if (WIN32)
    install(TARGETS chessboard RUNTIME DESTINATION .)
else()
    install(TARGETS chessboard)
endif()

add_custom_target(license SOURCES LICENSE.txt)
