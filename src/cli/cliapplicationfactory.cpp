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
#include "clioptions.h"
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

Options *CliApplicationFactory::createOptions()
{
    return new CliOptions();
}

bool CliApplicationFactory::processOptions(QCommandLineParser *parser, Options *options, QString *errorMessage)
{
    bool success = ApplicationFactoryBase::processOptions(parser, options, errorMessage);
    if (!success)
        return false;
    CliOptions *cliOptions = static_cast<CliOptions *>(options);
    if (parser->isSet(m_discoverOption))
        cliOptions->action = CliOptions::Action::Discover;
    else if (parser->isSet(m_getFenOption))
        cliOptions->action = CliOptions::Action::GetFen;
    else if (!parser->value(m_sendFenOption).isNull())
        cliOptions->action = CliOptions::Action::SendFen;
    cliOptions->quiet = parser->isSet(m_quietOption);
    QString address = parser->value(m_addressOption);
    if (!address.isNull()) {
        BoardAddress parsedAddress = BoardAddress::fromString(address);
        if (!parsedAddress.isValid()) {
            *errorMessage = QCoreApplication::translate("main", "%1: unable to parse address").arg(address);
            return false;
        }
        cliOptions->address = parsedAddress;
    }
    QString fen = parser->value(m_sendFenOption);
    if (!fen.isNull()) {
        BoardState state = BoardState::fromFenString(fen);
        if (!state.isValid()) {
            *errorMessage = QCoreApplication::translate("main", "%1: unable to parse FEN record").arg(fen);
            return false;
        }
        cliOptions->fenToSend = state;
    }
    return true;
}

ApplicationBase *CliApplicationFactory::create(const Options *options)
{
    const CliOptions *cliOptions = static_cast<const CliOptions *>(options);
    std::unique_ptr<CliApplicationBase> app;
    switch (cliOptions->action) {
    case CliOptions::Action::Discover:
        app.reset(new DiscoverApplication(cliOptions));
        break;
    case CliOptions::Action::GetFen:
        app.reset(new GetFenApplication(cliOptions));
        break;
    case CliOptions::Action::SendFen:
        app.reset(new SendFenApplication(cliOptions));
        break;
    case CliOptions::Action::Listen:
        app.reset(new ListenApplication(cliOptions));
        break;
    }
    return app.release();
}
