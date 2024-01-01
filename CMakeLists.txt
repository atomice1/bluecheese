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

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Bluetooth LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Bluetooth LinguistTools)

set(DEFAULT_STOCKFISH_PATH "" CACHE FILEPATH "Default path to stockfish executable")

if ("${DEFAULT_STOCKFISH_PATH}" STREQUAL "")
    message(FATAL_ERROR "The AI support in Bluecheese requires the Stockfish engine. You must set DEFAULT_STOCKFISH_PATH to the path to the stockfish executable, or \"none\" to disable AI support.")
endif()

configure_file(config.h.in config.h)
include_directories("${CMAKE_BINARY_DIR}")

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

add_custom_target(docs SOURCES
    README.md
    LICENSE.txt)
