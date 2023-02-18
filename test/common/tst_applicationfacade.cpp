/*
 * bluecheese
 * Copyright (C) 2023 Chris January
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

#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include "applicationfacade.h"
#include "chessboard.h"

using namespace Chessboard;

class MockRemoteBoard : public RemoteBoard
{
public:
    MockRemoteBoard(QObject *parent = nullptr) :
        RemoteBoard(BoardAddress(), parent) {
    }
};

class MockApplicationFacade : public ApplicationFacade
{
public:
    MockApplicationFacade(QObject *parent = nullptr) :
        ApplicationFacade(parent) {}
    ConnectionManager *connectionManager()
    {
        return m_connectionManager;
    }
};

template<typename T>
class AutoDeletePointer : public QPointer<T>
{
public:
    AutoDeletePointer(T *ptr) : QPointer<T>(ptr) {}
    ~AutoDeletePointer() { if (*this) delete *this; }
};

class TestApplicationFacade: public QObject
{
    Q_OBJECT
private slots:
    void connectDisconnect()
    {
        MockApplicationFacade appFacade;
        AutoDeletePointer<MockRemoteBoard> board = new MockRemoteBoard;
        QSignalSpy connectedSpy(&appFacade, &MockApplicationFacade::connected);
        emit appFacade.connectionManager()->connected(board);
        QCOMPARE(connectedSpy.count(), 1);
        QSignalSpy disconnectedSpy(&appFacade, &MockApplicationFacade::disconnected);
        emit appFacade.connectionManager()->disconnected();
        QCOMPARE(disconnectedSpy.count(), 1);
    }
};

QTEST_MAIN(TestApplicationFacade)

#include "tst_applicationfacade.moc"
