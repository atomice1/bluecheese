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

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)

set(TS_FILES ${CMAKE_SOURCE_DIR}/ts/chessboard-desktop_en_GB.ts)

add_executable(chessboard-desktop
    Info.plist
    chessboardscene.cpp
    chessboardscene.h
    connectingdialog.cpp
    connectingdialog.h
    connectingdialog.ui
    desktopapplication.cpp
    desktopapplication.h
    desktopapplicationfactory.cpp
    desktopapplicationfactory.h
    desktopguifacade.cpp
    desktopguifacade.h
    discoverydialog.cpp
    discoverydialog.h
    discoverydialog.ui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    promotiondialog.cpp
    promotiondialog.h
    promotiondialog.ui
    syncdialog.cpp
    syncdialog.h
    syncdialog.ui)

target_link_libraries(chessboard-desktop
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        chessboard
    PRIVATE
        chessboard-common)

if(WIN32)
    set_target_properties(chessboard-desktop PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(chessboard-desktop PROPERTIES MACOSX_BUNDLE TRUE)
    set_target_properties(chessboard-desktop PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
    target_link_options(chessboard-desktop PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif()

set(APP_NAME bluecheese-gui)
set_target_properties(chessboard-desktop PROPERTIES OUTPUT_NAME ${APP_NAME})

# From https://github.com/miurahr/cmake-qt-packaging-example/blob/master/Packaging.cmake (MIT license)
get_target_property(qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${qmake_executable}" DIRECTORY)

if(WIN32)
    install(TARGETS chessboard-desktop RUNTIME DESTINATION .)
    install(TARGETS chessboard LIBRARY DESTINATION .)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    install(CODE "execute_process(COMMAND ${WINDEPLOYQT_EXECUTABLE} ${APP_NAME}.exe -verbose=1 -bluetooth WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})")
elseif (APPLE)
    install(TARGETS chessboard-desktop BUNDLE DESTINATION .)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
    set_target_properties(chessboard-desktop PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")
    install(TARGETS chessboard LIBRARY DESTINATION ${APP_NAME}.app/Contents/Frameworks)
    install(CODE "execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} ${APP_NAME}.app -verbose=1 -libpath=${CMAKE_BINARY_DIR}/src/lib WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})")
    install(CODE "file(GLOB DYLIBS ${CMAKE_INSTALL_PREFIX}/${APP_NAME}.app/Contents/Frameworks/*)\nfile(COPY \${DYLIBS} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)")
else()
    install(TARGETS chessboard-desktop)
endif()

if(COMMAND qt_add_lupdate)
    qt_add_translations(chessboard-desktop
        TS_FILES ${TS_FILES})
else()
    qt6_add_translations(chessboard-desktop
        TS_FILES ${TS_FILES})
endif()

add_custom_target(chessboard-desktop-ts SOURCES ${TS_FILES})
