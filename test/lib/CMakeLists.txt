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

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

add_executable(tst_boardstate
    tst_boardstate.cpp

)
add_test(NAME boardstate COMMAND tst_boardstate)

target_link_libraries(tst_boardstate
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        chessboard)

add_executable(tst_algebraicnotation
    tst_algebraicnotation.cpp
)
add_test(NAME algebraicnotation COMMAND tst_algebraicnotation)

target_link_libraries(tst_algebraicnotation
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        chessboard)

add_executable(tst_pgn
    tst_pgn.cpp
    tst_pgn.cpp
)
add_test(NAME pgn COMMAND tst_pgn)

target_link_libraries(tst_pgn
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        chessboard)

if(WIN32)
    get_target_property(qt_core_location Qt${QT_VERSION_MAJOR}::Core IMPORTED_LOCATION)
    get_filename_component(qt_core_path "${qt_core_location}" PATH)
    string(REPLACE "/" "\\" qt_core_path_bs "${qt_core_path}")
    string(REPLACE "/" "\\" chessboard_location_bs $<TARGET_FILE_DIR:chessboard>)
    string(REPLACE ";" "\;" escaped_path "$ENV{PATH}")
    string(PREPEND path "${chessboard_location_bs}" "\;" "${qt_core_path_bs}" "\;" "${escaped_path}")
    set_property(TEST
        algebraicnotation
        boardstate
        pgn
        APPEND PROPERTY ENVIRONMENT
        "PATH=${path}")
endif()
