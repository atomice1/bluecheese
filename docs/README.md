Bluecheese Smart Chessboard Client
==================================

bluecheese is an unofficial client for the ChessUp smart chessboard.

Supported Platforms: Windows 10 22H2, Windows 11, MacOS 13, MacOS 14, Ubuntu 22.04, Ubuntu 24.04.

Building
--------

Requirements: Qt SDK 6.6, Qt Connectivity, cmake 3.21, Stockfish 16 (for AI support)

Ubuntu packages: libglx-dev libgl1-mesa-dev

    cmake -DDEFAULT_STOCKFISH_PATH=<path to stockfish executable> .
    cmake --build .
    cmake --install . --prefix=<directory to install to>

Desktop
-------

Run `bluecheese-gui` to start the desktop GUI app.

Play against another human player (over the board, or in the app) or the Stockfish engine. Choose your assistance level 1-6.

*Edit mode* allows you to fix the piece positions when they don't match the physical board.

Online play is not supported.

Command Line
------------

Show addresses of available boards:

    bluecheese --discover

Print a new FEN string after every move:

    bluecheese --address ADDRESS --listen

Send a FEN string to the board:

    bluecheese --address ADDRESS --sendfen FEN

Get a FEN string from the board:

    bluecheese --address ADDRESS --getfen
