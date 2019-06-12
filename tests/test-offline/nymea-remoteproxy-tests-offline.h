/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NYMEA_REMOTEPROXY_TESTS_OFFLINE_H
#define NYMEA_REMOTEPROXY_TESTS_OFFLINE_H

#include "basetest.h"

using namespace remoteproxy;
using namespace remoteproxyclient;

class RemoteProxyOfflineTests : public BaseTest
{
    Q_OBJECT
public:
    explicit RemoteProxyOfflineTests(QObject *parent = nullptr);
    ~RemoteProxyOfflineTests() = default;

private slots:
    // Basic stuff
    void startStopServer();
    void dummyAuthenticator();
    void monitorServer();

    void configuration_data();
    void configuration();

    // WebSocket connection
    void serverPortBlocked();
    void websocketBinaryData();
    void websocketPing();

    // WebSocket connection API
    void getIntrospect();
    void getHello();

    void apiBasicCalls_data();
    void apiBasicCalls();

    void authenticate_data();
    void authenticate();

    void authenticateNonce();
    void authenticateSendData();

    // Client lib
    void clientConnectionTcpSocket();
    void clientConnectionWebSocket();
    void remoteConnection();
    void multipleRemoteConnection();
    void trippleConnection();
    void duplicateUuid();
    void sslConfigurations();

    void inactiveTimeout();
    void jsonRpcTimeout();
    void authenticationReplyTimeout();
    void authenticationReplyConnection();

    // TCP Websocket combinations
    //void tcpRemoteConnection();

};

#endif // NYMEA_REMOTEPROXY_TESTS_OFFLINE_H
