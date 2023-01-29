# bluecheese
# Copyright (C) 2022-2023 Chris January
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set(TS_FILES ${CMAKE_SOURCE_DIR}/ts/chessboard-cli_en_GB.ts)

get_target_property(chessboard_common_translation_qrc chessboard-common _qt_generated_qrc_files)

add_executable(chessboard-cli
  cliapplicationbase.cpp
  cliapplicationbase.h
  cliapplicationfactory.cpp
  cliapplicationfactory.h
  connectedcliapplicationbase.cpp
  connectedcliapplicationbase.h
  discoverapplication.cpp
  discoverapplication.h
  getfenapplication.cpp
  getfenapplication.h
  listenapplication.cpp
  listenapplication.h
  main.cpp
  sendfenapplication.cpp
  sendfenapplication.h
  Info.plist
  #${chessboard_common_translation_qrc}
)

target_link_libraries(chessboard-cli
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        chessboard
    PRIVATE
        chessboard-common)

if (APPLE)
    set_target_properties(chessboard-cli PROPERTIES INSTALL_RPATH "@executable_path/../lib")
    target_link_options(chessboard-cli PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif()

set_target_properties(chessboard-cli PROPERTIES OUTPUT_NAME bluecheese)

if(WIN32)
    install(TARGETS chessboard-cli RUNTIME DESTINATION .)
else()
    install(TARGETS chessboard-cli)
endif()

if(COMMAND qt_add_lupdate)
    qt_add_translations(chessboard-cli
        TS_FILES ${TS_FILES})
else()
    qt6_add_translations(chessboard-cli
        TS_FILES ${TS_FILES})
endif()

add_custom_target(chessboard-cli-ts SOURCES ${TS_FILES})
