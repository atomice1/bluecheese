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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "aicontroller.h"
#include "aiplayerfactory.h"
#include "applicationfacade.h"
#include "commontranslations.h"
#include "compositeboard.h"
#include "stockfishaiplayer.h"
#include "config.h"

using namespace Chessboard;

namespace {
    const QLatin1String ORGANIZATION_NAME("atomice.com");
    const QLatin1String APPLICATION_NAME("chessboard");
    const QLatin1String CONNECTION_GROUP("connection");
    const QLatin1String ADDRESS("address");
    const QLatin1String STOCKFISH_GROUP("stockfish");
    const QLatin1String PATH("path");
}

ApplicationFacade::ApplicationFacade(AiPlayerFactory *aiPlayerFactory, QObject *parent)
    : QObject{parent}
{
    construct(aiPlayerFactory);
}

class StockfishAiPlayerFactory : public AiPlayerFactory
{
public:
    StockfishAiPlayerFactory(const QString& stockfishPath) :
        m_stockfishPath(stockfishPath)
    {
    }
    AiPlayer *createAiPlayer(Chessboard::Colour colour, QObject *parent = nullptr)
    {
        return new StockfishAiPlayer(colour, m_stockfishPath, parent);
    }
private:
    QString m_stockfishPath;
};

ApplicationFacade::ApplicationFacade(QObject *parent)
    : QObject{parent}
{
    m_settings.beginGroup(STOCKFISH_GROUP);
    m_stockfishPath = m_settings.value(PATH, QString()).toString();
    if (m_stockfishPath.isEmpty())
        m_stockfishPath = QFile::decodeName(DEFAULT_STOCKFISH_PATH);
    if (m_stockfishPath == QLatin1String("none"))
        m_stockfishPath.clear();
    else if (QFileInfo(m_stockfishPath).isRelative())
        m_stockfishPath = QFileInfo(QDir(QCoreApplication::applicationDirPath()), m_stockfishPath).filePath();
    m_settings.endGroup();
    StockfishAiPlayerFactory stockfishAiPlayerFactory(m_stockfishPath);
    construct(&stockfishAiPlayerFactory);
}

void ApplicationFacade::construct(AiPlayerFactory *aiPlayerFactory)
{
    m_boardDiscovery = new BoardDiscovery(this);
    connect(m_boardDiscovery, &BoardDiscovery::finished, this, &ApplicationFacade::discoveryFinished);
    connect(m_boardDiscovery, &BoardDiscovery::error, this, &ApplicationFacade::onDiscoveryError);
    connect(m_boardDiscovery, &BoardDiscovery::boardDiscovered, this, &ApplicationFacade::boardDiscovered);
    connect(m_boardDiscovery, &BoardDiscovery::boardsDiscovered, this, &ApplicationFacade::boardsDiscovered);
    connect(m_boardDiscovery, &BoardDiscovery::noBoardsDiscovered, this, &ApplicationFacade::noBoardsDiscovered);
    connect(m_boardDiscovery, &BoardDiscovery::started, this, &ApplicationFacade::discoveryStarted);

    m_connectionManager = new ConnectionManager(this);
    connect(m_connectionManager, &ConnectionManager::connected, this, [this](RemoteBoard *board) {
        m_board->setRemoteBoard(board);
        emit connected(board);
    });
    connect(m_connectionManager, &ConnectionManager::disconnected, this, [this]() {
        qDebug("ApplicationFacade::disconnected");
        emit disconnected();
        m_board->setRemoteBoard(nullptr);
    });
    connect(m_connectionManager, &ConnectionManager::error, this, &ApplicationFacade::onConnectionError);
    connect(m_connectionManager, &ConnectionManager::connectionFailed, this, &ApplicationFacade::connectionFailed);
    connect(m_connectionManager, &ConnectionManager::connecting, this, &ApplicationFacade::connecting);

    m_board = new CompositeBoard(this);
    connect(m_board, &CompositeBoard::remoteMove, this, &ApplicationFacade::remoteMove);
    connect(m_board, &CompositeBoard::remoteUndo, this, &ApplicationFacade::remoteUndo);
    connect(m_board, &CompositeBoard::remoteBoardState, this, &ApplicationFacade::remoteBoardState);
    connect(m_board, &CompositeBoard::localOutOfSyncWithRemote, this, &ApplicationFacade::localOutOfSyncWithRemote);
    connect(m_board, &CompositeBoard::illegalMove, this, &ApplicationFacade::illegalMove);
    connect(m_board, &CompositeBoard::remoteOutOfSyncWithLocal, this, &ApplicationFacade::remoteOutOfSyncWithLocal);
    connect(m_board, &CompositeBoard::canUndoChanged, this, &ApplicationFacade::canUndoChanged);
    connect(m_board, &CompositeBoard::boardStateChanged, this, [this](const BoardState& state) {
        m_aiController->cancel();
        emit boardStateChanged(state);
        if (m_gameProgress.state != GameProgress::InProgress)
            emit gameProgressChanged(GameProgress(GameProgress::InProgress));
        if (!m_board->isPromotionRequired())
            emit activeColourChanged(state.activeColour);
    });
    connect(m_board, &CompositeBoard::promotionRequired, this, [this]() {
        if (isCurrentPlayerAppHuman())
            emit promotionRequired();
        else if (isCurrentPlayerAppAi())
            m_aiController->promotionRequired(m_board->activeColour());
    });
    connect(m_board, &CompositeBoard::drawRequested, this, [this](Colour requestor) {
        Chessboard::Colour invertedColour = invertColour(requestor);
        if (isPlayerAppHuman(invertedColour))
            emit drawRequested(requestor);
        else if (isPlayerAppAi(invertedColour))
            m_aiController->drawRequested(requestor);
    });
    connect(m_board, &CompositeBoard::drawDeclined, this, [this](Colour declinor) {
        Chessboard::Colour invertedColour = invertColour(declinor);
        if (isPlayerAppHuman(invertedColour)) {
            emit drawDeclined(declinor);
        } else if (isPlayerAppAi(invertedColour)) {
            m_aiController->drawDeclined(declinor);
        }
    });
    connect(m_board, &CompositeBoard::resignation, this, [this](Colour colour) {
        emit gameProgressChanged(GameProgress(GameProgress::Resignation,
                                              (colour == Colour::White) ? Colour::Black : Colour::White));
        emit gameOver();
        emit resignation(colour);
    });
    connect(m_board, &CompositeBoard::checkmate, this, [this](Colour colour) {
        emit gameProgressChanged(GameProgress(GameProgress::Checkmate, colour));
        emit gameOver();
        emit checkmate(colour);
    });
    connect(m_board, &CompositeBoard::draw, this, [this](DrawReason reason) {
        emit gameProgressChanged(GameProgress(GameProgress::Draw, reason));
        emit gameOver();
        emit draw(reason);
    });

    m_aiController = new AiController(aiPlayerFactory, this);
    // It is important we allow the game progress to be updated before maybe starting the AI engine.
    connect(this, &ApplicationFacade::activeColourChanged, this, [this](Chessboard::Colour colour) {
        qDebug("ApplicationFacade::activeColourChanged");
        QMetaObject::invokeMethod(this, [this, colour]() {
                ApplicationFacade::maybeStartAi(colour);
            }, Qt::QueuedConnection);
    });
    connect(this, &ApplicationFacade::gameOver, this, [this]() {
        m_aiController->cancel();
    });
    connect(m_aiController, &AiController::requestMove, this, &ApplicationFacade::requestMove);
    connect(m_aiController, &AiController::requestDraw, this, &ApplicationFacade::requestDraw);
    connect(m_aiController, &AiController::declineDraw, this, &ApplicationFacade::declineDraw);
    connect(m_aiController, &AiController::requestResignation, this, &ApplicationFacade::requestResignation);
    connect(m_aiController, &AiController::requestPromotion, this, &ApplicationFacade::requestPromotion);
    connect(m_aiController, &AiController::assistance, this, [this](const QList<Chessboard::AssistanceColour>& colours) {
        qDebug("ApplicationFacade::assistance");
        m_board->sendAssistance(colours);
        emit assistance(colours);
    });
    connect(m_aiController, &AiController::error, this, &ApplicationFacade::aiError);

    connect(this, &ApplicationFacade::gameProgressChanged, this, [this](GameProgress gameProgress) {
        m_gameProgress = gameProgress;
    });
    connect(this, &ApplicationFacade::gameOptionsChanged, this, [this](const GameOptions& gameOptions) {
        if (gameOptions.white.playerType == Chessboard::PlayerType::Ai)
            m_aiController->setStrength(Colour::White, gameOptions.white.aiNominalElo);
        else
            m_aiController->setAssistanceLevel(Colour::White, gameOptions.white.assistanceLevel);
        if (gameOptions.black.playerType == Chessboard::PlayerType::Ai)
            m_aiController->setStrength(Colour::Black, gameOptions.black.aiNominalElo);
        else
            m_aiController->setAssistanceLevel(Colour::Black, gameOptions.black.assistanceLevel);
    });

    m_settings.beginGroup(CONNECTION_GROUP);
    m_lastConnectedAddress = BoardAddress::fromByteArray(m_settings.value(ADDRESS, QByteArray()).toByteArray());
    m_settings.endGroup();

    Chessboard::Colour colour = m_board->activeColour();
    QMetaObject::invokeMethod(this, [this, colour]() {
            ApplicationFacade::maybeStartAi(colour);
        }, Qt::QueuedConnection);
}
void ApplicationFacade::connectToLast()
{
    qDebug("ApplicationFacade::connectToLast()");
    if (m_lastConnectedAddress.isValid())
        m_connectionManager->connectToBoard(m_lastConnectedAddress, m_board);
    else
        emit noLastConnectedAddress();
}

void ApplicationFacade::connectToLastOrDiscover()
{
    qDebug("ApplicationFacade::connectToLastOrDiscovery()");
    if (m_lastConnectedAddress.isValid())
        m_connectionManager->connectToBoard(m_lastConnectedAddress, m_board);
    else
        m_boardDiscovery->start();
}

void ApplicationFacade::connectToBoard(const BoardAddress& address)
{
    qDebug("ApplicationFacade::connectToBoard(%s)", qPrintable(address.toString()));
    setLastConnectedAddress(address);
    m_connectionManager->connectToBoard(address, this);
}

void ApplicationFacade::startDiscovery()
{
    qDebug("ApplicationFacade::startDiscovery");
    m_boardDiscovery->start();
}

void ApplicationFacade::stopDiscovery()
{
    qDebug("ApplicationFacade::stopDiscovery");
    m_boardDiscovery->stop();
}

void ApplicationFacade::disconnectFromBoard()
{
    qDebug("ApplicationFacade::disconnectFromBoard");
    m_connectionManager->disconnectFromBoard();
}

void ApplicationFacade::onDiscoveryError(BoardDiscovery::Error error)
{
    qDebug("ApplicationFacade::discoveryError(%d)", error);
    QString errorMessage;
    switch (error) {
    case BoardDiscovery::PoweredOffError:
        errorMessage = tr("Bluetooth powered off / radio disabled");
        break;
    case BoardDiscovery::InputOutputError:
        errorMessage = tr("input/output error");
        break;
    case BoardDiscovery::InvalidBluetoothAdapterError:
        errorMessage = tr("invalid Bluetooth adapter");
        break;
    case BoardDiscovery::UnsupportedPlatformError:
        errorMessage = tr("unsupported platform");
        break;
    case BoardDiscovery::UnsupportedDiscoveryMethod:
        errorMessage = tr("unsupported discovery method");
        break;
    case BoardDiscovery::LocationServiceTurnedOffError:
        errorMessage = tr("Location Services turned off / disabled");
        break;
    case BoardDiscovery::MissingPermissionsError:
        errorMessage = tr("missing permissions");
        break;
    default:
        errorMessage = tr("unknown error");
        break;
    }
    emit this->error(errorMessage);
}

void ApplicationFacade::onConnectionError(ConnectionManager::Error error)
{
    qDebug("ApplicationFacade::onConnectionError(%d)", error);
    QString errorMessage;
    switch (error) {
    case ConnectionManager::UnknownRemoteDeviceError:
        errorMessage = tr("unknown remote device");
        break;
    case ConnectionManager::NetworkError:
        errorMessage = tr("network error");
        break;
    case ConnectionManager::InvalidBluetoothAdapterError:
        errorMessage = tr("invalid Bluetooth adapter");
        break;
    case ConnectionManager::ConnectionError:
        errorMessage = tr("connection error");
        break;
    case ConnectionManager::AdvertisingError:
        errorMessage = tr("advertising error");
        break;
    case ConnectionManager::RemoteHostClosedError:
        errorMessage = tr("remote host closed");
        break;
    case ConnectionManager::AuthorizationError:
        errorMessage = tr("authorization error");
        break;
    case ConnectionManager::MissingPermissionsError:
        errorMessage = tr("missing permissions");
        break;
    case ConnectionManager::DisconnectDuringServiceDiscovery:
        errorMessage = tr("disconnected during service discovery");
        break;
    case ConnectionManager::DisconnectDuringCharacteristicDiscovery:
        errorMessage = tr("disconnected during characteristic discovery");
        break;
    case ConnectionManager::NotSupported:
        errorMessage = tr("device is not supported");
        break;
    default:
        errorMessage = tr("unknown error");
        break;
    }
    emit this->error(errorMessage);
}

void ApplicationFacade::setLastConnectedAddress(const Chessboard::BoardAddress& address)
{
    qDebug("ApplicationFacade::setLastConnectedAddress(%s)", qPrintable(address.toString()));
    m_lastConnectedAddress = address;
    m_settings.beginGroup(CONNECTION_GROUP);
    m_settings.setValue(ADDRESS, m_lastConnectedAddress.toByteArray());
    m_settings.endGroup();
}

void ApplicationFacade::setBoardStateFromFen(const QString& fen)
{
    qDebug("ApplicationFacade::setBoardStateFromFen(%s)", qPrintable(fen));
    BoardState state = BoardState::fromFenString(fen);
    if (!state.isValid()) {
        emit invalidFenString(fen);
    } else {
        if (m_board)
            m_board->setBoardState(state);
    }
}

void ApplicationFacade::requestRemoteBoardState()
{
    qDebug("ApplicationFacade::requestRemoteBoardState()");
    m_board->requestRemoteBoardState();
}

void ApplicationFacade::sendLocalBoardState()
{
    qDebug("ApplicationFacade::sendLocalBoardState()");
    m_board->sendLocalBoardState();
}

void ApplicationFacade::requestNewGame(const Chessboard::GameOptions& gameOptions)
{
    qDebug("ApplicationFacade::requestNewGame(...)");
    m_gameOptions = gameOptions;
    emit gameOptionsChanged(m_gameOptions);
    m_board->requestNewGame(gameOptions);
}

void ApplicationFacade::requestMove(int fromRow, int fromCol, int toRow, int toCol)
{
    qDebug("ApplicationFacade::requestMove(%d, %d, %d, %d)", fromRow, fromCol, toRow, toCol);
    m_board->requestMove(fromRow, fromCol, toRow, toCol);
}

void ApplicationFacade::requestUndo()
{
    qDebug("ApplicationFacade::requestUndo");
    m_board->requestUndo();
}

void ApplicationFacade::requestDraw(Chessboard::Colour requestor)
{
    qDebug("ApplicationFacade::requestDraw(%s)", (requestor == Colour::White) ? "white" : "black");
    m_board->requestDraw(requestor);
}

void ApplicationFacade::declineDraw(Chessboard::Colour declinor)
{
    qDebug("ApplicationFacade::declineDraw(%s)", (declinor == Colour::White) ? "white" : "black");
    m_board->declineDraw(declinor);
}

void ApplicationFacade::requestResignation(Chessboard::Colour requestor)
{
    qDebug("ApplicationFacade::requestResignation(%s)", (requestor == Colour::White) ? "white" : "black");
    m_board->requestResignation(requestor);
}

void ApplicationFacade::requestPromotion(Chessboard::Piece piece)
{
    qDebug("ApplicationFacade::requestPromotion(%s)",
           qPrintable(pieceToString(piece)));
    m_board->requestPromotion(piece);
}

bool ApplicationFacade::isCurrentPlayerAppAi() const
{
    return isPlayerAppAi(m_board->activeColour());
}

bool ApplicationFacade::isPlayerAppHuman(Colour colour) const
{
    const PlayerOptions& playerOptions =
        colour == Colour::Black ? m_gameOptions.black : m_gameOptions.white;
    return playerOptions.playerType == PlayerType::Human &&
           playerOptions.playerLocation == PlayerLocation::LocalApp;
}

bool ApplicationFacade::isCurrentPlayerAppHuman() const
{
    return isPlayerAppHuman(m_board->activeColour());
}

bool ApplicationFacade::isPlayerAppAi(Colour colour) const
{
    const PlayerOptions& playerOptions =
        colour == Colour::Black ? m_gameOptions.black : m_gameOptions.white;
    return playerOptions.playerType == PlayerType::Ai &&
           playerOptions.playerLocation == PlayerLocation::LocalApp;
}

void ApplicationFacade::aiError(AiPlayer::Error error)
{
    qDebug("ApplicationFacade::aiError(%d)", error);
    QString errorMessage;
    switch (error) {
    case AiPlayer::EngineNotConfigured:
        errorMessage = tr("engine not configured");
        emit engineNeedsConfigure(errorMessage, m_stockfishPath);
        return;
    case AiPlayer::EngineNotFound:
        errorMessage = tr("engine not found");
        emit engineNeedsConfigure(errorMessage, m_stockfishPath);
        return;
    case AiPlayer::EngineIncompatible:
        errorMessage = tr("engine is incompatible");
        break;
    case AiPlayer::EngineNoStart:
        errorMessage = tr("engine executable did not start properly");
        break;
    case AiPlayer::EngineNoBoot:
        errorMessage = tr("engine did not boot properly");
        break;
    case AiPlayer::EngineTimedOut:
        errorMessage = tr("engine timed out");
        break;
    case AiPlayer::UnknownError:
    default:
        errorMessage = tr("unknown engine error");
        break;
    }
    emit this->error(errorMessage);
}

void ApplicationFacade::setGameOptions(const Chessboard::GameOptions& gameOptions)
{
    qDebug("ApplicationFacade::setGameOptions(...");
    bool oldAppAi = isPlayerAppAi(m_board->activeColour());
    m_gameOptions = gameOptions;
    m_board->setGameOptions(gameOptions);
    emit gameOptionsChanged(gameOptions);
    if (isPlayerAppAi(m_board->activeColour()) != oldAppAi)
        maybeStartAi(m_board->activeColour());
}

void ApplicationFacade::maybeStartAi(Chessboard::Colour colour)
{
    if (m_gameProgress.state == GameProgress::InProgress) {
        m_aiController->cancel();
        if (isPlayerAppAi(colour)) {
            qDebug("ApplicationFacade::maybeStartAi: start AI");
            m_aiController->start(colour, m_board->boardState());
        } else {
            qDebug("ApplicationFacade::maybeStartAi: start assistance");
            m_aiController->startAssistance(colour, m_board->boardState());
        }
    } else {
        qDebug("ApplicationFacade::maybeStartAi: game not in progress");
    }
}

void ApplicationFacade::configureEngine(const QString& stockfishPath)
{
    m_stockfishPath = stockfishPath;
    m_settings.beginGroup(STOCKFISH_GROUP);
    m_settings.setValue(PATH, stockfishPath);
    m_settings.endGroup();
    StockfishAiPlayerFactory stockfishAiPlayerFactory(stockfishPath);
    m_aiController->setFactory(&stockfishAiPlayerFactory);
    maybeStartAi(m_board->activeColour());
}
