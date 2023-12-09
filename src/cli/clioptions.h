#ifndef CLIOPTIONS_H
#define CLIOPTIONS_H

#include "chessboard.h"
#include "options.h"

struct CliOptions : public Options {
    enum class Action {
        Listen,
        Discover,
        GetFen,
        SendFen
    };

    bool quiet {false};
    Action action {Action::Listen};
    Chessboard::BoardAddress address;
    Chessboard::BoardState fenToSend;
};

#endif // CLIOPTIONS_H
