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

set(TS_FILES ${CMAKE_SOURCE_DIR}/ts/chessboard-common_en_GB.ts)

add_library(chessboard-common STATIC
            applicationbase.cpp
            applicationbase.h
            applicationfacade.cpp
            applicationfacade.h
            applicationfactorybase.cpp
            applicationfactorybase.h
            commontranslations.cpp
            commontranslations.h
            compositeboard.cpp
            compositeboard.h
            connectionstate.cpp
            connectionstate.h
            gameprogress.cpp
            gameprogress.h
            guiapplicationbase.cpp
            guiapplicationbase.h
            guifacade.cpp
            guifacade.h
            options.h
            version.h)

target_include_directories(chessboard-common PUBLIC .)

target_link_libraries(chessboard-common
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        chessboard)

if(COMMAND qt_add_lupdate)
    qt_add_translations(chessboard-common
        TS_FILES ${TS_FILES}
        QM_FILES_OUTPUT_VARIABLE QM_FILES)
    qt_add_resources(chessboard-common "common_translations"
        PREFIX "/i18n"
        BASE "${CMAKE_CURRENT_BINARY_DIR}"
	FILES ${QM_FILES})
else()
    qt6_add_translations(chessboard-common
        TS_FILES ${TS_FILES}
	QM_FILES_OUTPUT_VARIABLE QM_FILES)
    qt6_add_resources(chessboard-common "common_translations"
        PREFIX "/i18n"
        BASE "${CMAKE_CURRENT_BINARY_DIR}"
	FILES ${QM_FILES})
endif()

add_custom_target(chessboard-common-ts SOURCES ${TS_FILES})
