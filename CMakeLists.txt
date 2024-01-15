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

cmake_minimum_required(VERSION 3.16)

project(chessboard LANGUAGES CXX)

include(ExternalProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Bluetooth LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Bluetooth LinguistTools)

set(DEFAULT_STOCKFISH_PATH "stockfish" CACHE FILEPATH "Default path to stockfish executable")

ExternalProject_Add(stockfish
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    URL https://github.com/official-stockfish/Stockfish/archive/refs/tags/sf_16.zip
    URL_HASH "SHA512=4c1ee8e36a8786392409fd0fa85d3751f6595b69529df09db3011229019a3f76b410dc84a077774ec1095c8a7bcde529db4cd0766f103ce94fc59fc186e6ee89"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make -C <SOURCE_DIR>/src build
    INSTALL_COMMAND make -C <SOURCE_DIR>/src install PREFIX="${CMAKE_BINARY_DIR}"
    TEST_COMMAND "")
install(FILES "${CMAKE_BINARY_DIR}/bin/stockfish${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION bin)

configure_file(config.h.in config.h)
include_directories("${CMAKE_BINARY_DIR}")

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

add_custom_target(docs SOURCES
    docs/README.md
    LICENSE.txt)
