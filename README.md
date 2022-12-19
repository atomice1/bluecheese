Bluecheese Smart Chessboard Client
==================================

bluecheese is an unofficial client for the ChessUp smart chessboard.

Desktop
-------

Run `bluecheese-gui` to start the desktop GUI app.

Command Line
------------

Show addresses of available boards:

  bluecheese --discover

Print a new FEN string after every move:

  bluecheese --address ADDRESS --listen

Send a FEN string to the board:

  bluecheese --address ADDRESS --sendfen FEN

Get a FEN string from the board:

  bluecheese --address ADDRESS --getfen FEN
