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

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"
#include "nymea-remoteproxy-tests-online.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

RemoteProxyOnlineTests::RemoteProxyOnlineTests(QObject *parent) :
    BaseTest(parent)
{
    m_authenticator = qobject_cast<Authenticator *>(m_awsAuthenticator);
}

void RemoteProxyOnlineTests::awsStaticCredentials()
{
    startServer();

    // Connect to the server (insecure disabled)
    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    connect(connection, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect to server (insecue enabled for testing)
    QSignalSpy readySpy(connection, &RemoteProxyConnection::ready);
    QVERIFY(connection->connectServer(m_serverUrl));
    readySpy.wait();
    QVERIFY(readySpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(!connection->isRemoteConnected());
    QVERIFY(connection->state() == RemoteProxyConnection::StateReady);
    QVERIFY(connection->serverUrl() == m_serverUrl);
    QVERIFY(connection->connectionType() == RemoteProxyConnection::ConnectionTypeWebSocket);
    QVERIFY(connection->serverName() == SERVER_NAME_STRING);
    QVERIFY(connection->proxyServerName() == Engine::instance()->serverName());
    QVERIFY(connection->proxyServerVersion() == SERVER_VERSION_STRING);
    QVERIFY(connection->proxyServerApiVersion() == API_VERSION_STRING);

    QSignalSpy errorSpy(connection, &RemoteProxyConnection::errorOccured);
    QSignalSpy spyDisconnected(connection, &RemoteProxyConnection::disconnected);

    QVERIFY(connection->authenticate("foobar"));
    errorSpy.wait();
    QVERIFY(errorSpy.count() == 1);
    QVERIFY(connection->error() == QAbstractSocket::ProxyAuthenticationRequiredError);

    // Disconnect and clean up
    connection->disconnectServer();
    // FIXME: check why it waits the full time here
    spyDisconnected.wait(100);
    QVERIFY(spyDisconnected.count() == 1);
    QVERIFY(!connection->isConnected());

    connection->deleteLater();

    stopServer();
}

void RemoteProxyOnlineTests::awsDynamicCredentials()
{


}

QTEST_MAIN(RemoteProxyOnlineTests)
