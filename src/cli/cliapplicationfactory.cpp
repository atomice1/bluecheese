/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QIODevice>
#include <QTextStream>
#include "chessboard.h"
#include "cliapplicationfactory.h"
#include "discoverapplication.h"
#include "getfenapplication.h"
#include "listenapplication.h"
#include "sendfenapplication.h"

namespace {
    const QLatin1String APPLICATION_NAME("chessboard");
    const QLatin1String TRANSLATION_NAME("chessboard-cli");
}

using namespace Chessboard;

CliApplicationFactory::CliApplicationFactory() :
    ApplicationFactoryBase(APPLICATION_NAME, TRANSLATION_NAME),
    m_quietOption{"quiet", QCoreApplication::translate("main", "Quiet mode. Don't print informational output.")},
    m_discoverOption{"discover", QCoreApplication::translate("main", "Discover and print available boards.")},
    m_getFenOption{"getfen", QCoreApplication::translate("main", "Get FEN record for current board state.")},
    m_sendFenOption{"sendfen",
                    QCoreApplication::translate("main", "Send FEN record to board."),
                    QCoreApplication::translate("main", "FEN")},
    m_addressOption{"address",
                    QCoreApplication::translate("main", "Address of remote board to connect to."),
                    QCoreApplication::translate("main", "ADDRESS")}
{
}

void CliApplicationFactory::addCommandLineOptions(QCommandLineParser *parser)
{
    parser->addOption(m_quietOption);
    parser->addOption(m_discoverOption);
    parser->addOption(m_getFenOption);
    parser->addOption(m_sendFenOption);
    parser->addOption(m_addressOption);
}

bool CliApplicationFactory::validateArguments(QCommandLineParser *parser, QString *errorMessage)
{
    QString address = parser->value(m_addressOption);
    if (!address.isNull()) {
        BoardAddress parsedAddress = BoardAddress::fromString(address);
        if (!parsedAddress.isValid()) {
            *errorMessage = QCoreApplication::translate("main", "%1: unable to parse address").arg(address);
            return false;
        }
    }
    QString fen = parser->value(m_sendFenOption);
    if (!fen.isNull()) {
        BoardState state = BoardState::fromFenString(fen);
        if (!state.isValid()) {
            *errorMessage = QCoreApplication::translate("main", "%1: unable to parse FEN record").arg(fen);
            return false;
        }
    }
    return true;
}

ApplicationBase *CliApplicationFactory::create(QCommandLineParser *parser)
{
    std::unique_ptr<CliApplicationBase> app;
    if (parser->isSet(m_discoverOption)) {
        app.reset(new DiscoverApplication());
    } else {
        QString address = parser->value(m_addressOption);
        if (parser->isSet(m_getFenOption)) {
            app.reset(new GetFenApplication(address));
        } else if (!parser->value(m_sendFenOption).isNull()) {
            QString fen = parser->value(m_sendFenOption);
            app.reset(new SendFenApplication(address, fen));
        } else {
            app.reset(new ListenApplication(address));
        }
    }
    app->setQuiet(parser->isSet(m_quietOption));
    return app.release();
}
