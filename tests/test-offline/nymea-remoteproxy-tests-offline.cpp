/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymea-remoteproxy-tests-offline.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

RemoteProxyOfflineTests::RemoteProxyOfflineTests(QObject *parent) :
    BaseTest(parent)
{

}

void RemoteProxyOfflineTests::startStopServer()
{
    startServer();
    stopServer();
}

void RemoteProxyOfflineTests::dummyAuthenticator()
{
    cleanUpEngine();

    // Start proxy webserver
    Engine::instance()->setAuthenticator(m_dummyAuthenticator);
    QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->start(m_configuration);
    runningSpy.wait();

    QVERIFY(runningSpy.count() == 1);

    // Make sure the server is running
    QVERIFY(Engine::instance()->running());
    QVERIFY(Engine::instance()->webSocketServer()->running());
    QVERIFY(Engine::instance()->proxyServer()->running());

    // Create request
    QVariantMap params;
    params.insert("uuid", QUuid::createUuid().toString());
    params.insert("name", "test");
    params.insert("token", "foobar");

    QVariant response = invokeApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    verifyAuthenticationError(response);

    cleanUpEngine();
}

void RemoteProxyOfflineTests::monitorServer()
{
    startServer();



    stopServer();
}

void RemoteProxyOfflineTests::webserverConnectionBlocked()
{
    cleanUpEngine();

    // Create a dummy server which blocks the port
    QWebSocketServer dummyServer("dummy-server", QWebSocketServer::NonSecureMode);
    dummyServer.listen(QHostAddress::LocalHost, 1212);

    // Start proxy webserver
    QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->setAuthenticator(m_authenticator);
    Engine::instance()->start(m_configuration);
    runningSpy.wait();
    qDebug() << runningSpy.count();
    QVERIFY(runningSpy.count() == 1);

    // Make sure the server is not running
    QVERIFY(Engine::instance()->running());

    // Make sure the websocket server is not running
    QVERIFY(!Engine::instance()->webSocketServer()->running());

    QSignalSpy closedSpy(&dummyServer, &QWebSocketServer::closed);
    dummyServer.close();
    closedSpy.wait();
    QVERIFY(closedSpy.count() == 1);

    // Try again
    cleanUpEngine();
    startServer();

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::getIntrospect()
{
    // Start the server
    startServer();

    QVariant response = invokeApiCall("RemoteProxy.Introspect");
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::getHello()
{
    // Start the server
    startServer();

    QVariant response = invokeApiCall("RemoteProxy.Hello");
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::apiBasicCalls_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid call") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\"}") << true;
    QTest::newRow("missing id") << QByteArray("{\"method\":\"RemoteProxy.Hello\"}")<< false;
    QTest::newRow("missing method") << QByteArray("{\"id\":42}") << false;
    QTest::newRow("borked") << QByteArray("{\"id\":42, \"method\":\"RemoteProx")<< false;
    QTest::newRow("invalid function") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Explode\"}") << false;
    QTest::newRow("invalid namespace") << QByteArray("{\"id\":42, \"method\":\"ProxyRemote.Hello\"}") << false;
    QTest::newRow("missing dot") << QByteArray("{\"id\":42, \"method\":\"RemoteProxyHello\"}") << false;
    QTest::newRow("invalid params") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\", \"params\":{\"törööö\":\"chooo-chooo\"}}") << false;
}

void RemoteProxyOfflineTests::apiBasicCalls()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, valid);

    // Start the server
    startServer();

    QVariant response = injectSocketData(data);
    if (valid) {
        QVERIFY2(response.toMap().value("status").toString() == "success", "Call wasn't parsed correctly by nymea.");
    }

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::authenticate_data()
{
    QTest::addColumn<QString>("uuid");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("token");

    QTest::addColumn<int>("timeout");
    QTest::addColumn<Authenticator::AuthenticationError>("expectedError");

    QTest::newRow("success") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                             << 100 << Authenticator::AuthenticationErrorNoError;

    QTest::newRow("failed") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                            << 100 << Authenticator::AuthenticationErrorAuthenticationFailed;

    QTest::newRow("not responding") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                                    << 200 << Authenticator::AuthenticationErrorProxyError;

    QTest::newRow("aborted") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                             << 100 << Authenticator::AuthenticationErrorAborted;

    QTest::newRow("unknown") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                             << 100 << Authenticator::AuthenticationErrorUnknown;

}

void RemoteProxyOfflineTests::authenticate()
{
    QFETCH(QString, uuid);
    QFETCH(QString, name);
    QFETCH(QString, token);
    QFETCH(int, timeout);
    QFETCH(Authenticator::AuthenticationError, expectedError);

    // Start the server
    startServer();

    // Configure result
    m_mockAuthenticator->setExpectedAuthenticationError(expectedError);
    m_mockAuthenticator->setTimeoutDuration(timeout);

    // Create request
    QVariantMap params;
    params.insert("uuid", uuid);
    params.insert("name", name);
    params.insert("token", token);

    QVariant response = invokeApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    verifyAuthenticationError(response, expectedError);

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::clientConnection()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

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
    QVERIFY(connection->error() == RemoteProxyConnection::ErrorNoError);
    QVERIFY(connection->serverUrl() == m_serverUrl);
    QVERIFY(connection->connectionType() == RemoteProxyConnection::ConnectionTypeWebSocket);
    QVERIFY(connection->serverName() == SERVER_NAME_STRING);
    QVERIFY(connection->proxyServerName() == Engine::instance()->serverName());
    QVERIFY(connection->proxyServerVersion() == SERVER_VERSION_STRING);
    QVERIFY(connection->proxyServerApiVersion() == API_VERSION_STRING);

    QSignalSpy authenticatedSpy(connection, &RemoteProxyConnection::authenticated);
    QVERIFY(connection->authenticate("foobar"));
    authenticatedSpy.wait();
    QVERIFY(authenticatedSpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(connection->isAuthenticated());
    QVERIFY(connection->state() == RemoteProxyConnection::StateWaitTunnel);

    // Disconnect and clean up
    QSignalSpy spyDisconnected(connection, &RemoteProxyConnection::disconnected);
    connection->disconnectServer();
    // FIXME: check why it waits the full time here
    spyDisconnected.wait(100);

    QVERIFY(spyDisconnected.count() == 1);
    QVERIFY(!connection->isConnected());

    connection->deleteLater();
    stopServer();
}

void RemoteProxyOfflineTests::remoteConnection()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(QUuid::createUuid(), "Test client two", this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrl));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrl));
    connectionTwoReadySpy.wait();
    QVERIFY(connectionTwoReadySpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());

    // Authenticate one
    QSignalSpy remoteConnectionEstablishedOne(connectionOne, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionOne->authenticate(m_testToken));
    connectionOneAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(connectionOne->isConnected());
    QVERIFY(connectionOne->isAuthenticated());
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateWaitTunnel);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken));
    connectionTwoAuthenticatedSpy.wait();
    QVERIFY(connectionTwoAuthenticatedSpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());
    QVERIFY(connectionTwo->isAuthenticated());

    // Wait for both to be connected
    remoteConnectionEstablishedOne.wait(500);
    remoteConnectionEstablishedTwo.wait(500);

    QVERIFY(remoteConnectionEstablishedOne.count() == 1);
    QVERIFY(remoteConnectionEstablishedTwo.count() == 1);
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateRemoteConnected);
    QVERIFY(connectionTwo->state() == RemoteProxyConnection::StateRemoteConnected);

    // Pipe data trought the tunnel
    QSignalSpy remoteConnectionDataOne(connectionOne, &RemoteProxyConnection::dataReady);
    QSignalSpy remoteConnectionDataTwo(connectionTwo, &RemoteProxyConnection::dataReady);

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    connectionOne->sendData(dataOne);
    remoteConnectionDataTwo.wait(500);
    QVERIFY(remoteConnectionDataTwo.count() == 1);
    QCOMPARE(remoteConnectionDataTwo.at(0).at(0).toByteArray(), dataOne);

    // verify if data is the same

    connectionTwo->sendData(dataTwo);
    remoteConnectionDataOne.wait(500);
    QVERIFY(remoteConnectionDataOne.count() == 1);
    QCOMPARE(remoteConnectionDataOne.at(0).at(0).toByteArray(), dataTwo);

    // verify if data is the same

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::trippleConnection()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(QUuid::createUuid(), "Test client two", this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionThree = new RemoteProxyConnection(QUuid::createUuid(), "Token thief ^w^", this);
    connect(connectionThree, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrl));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrl));
    connectionTwoReadySpy.wait();
    QVERIFY(connectionTwoReadySpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());

    // Connect two
    QSignalSpy connectionThreeReadySpy(connectionThree, &RemoteProxyConnection::ready);
    QVERIFY(connectionThree->connectServer(m_serverUrl));
    connectionThreeReadySpy.wait();
    QVERIFY(connectionThreeReadySpy.count() == 1);
    QVERIFY(connectionThree->isConnected());


    // Authenticate one
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionOne->authenticate(m_testToken));
    connectionOneAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(connectionOne->isConnected());
    QVERIFY(connectionOne->isAuthenticated());
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateWaitTunnel);

    QSignalSpy remoteConnectionEstablishedOne(connectionOne, &RemoteProxyConnection::remoteConnectionEstablished);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken));
    connectionTwoAuthenticatedSpy.wait();
    QVERIFY(connectionTwoAuthenticatedSpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());
    QVERIFY(connectionTwo->isAuthenticated());

    // Wait for both to be connected
    remoteConnectionEstablishedOne.wait(500);

    // Now connect a third connection and make sure it gets disconnected

    // Authenticate three
    QSignalSpy connectionThreeDisconnectSpy(connectionThree, &RemoteProxyConnection::disconnected);
    QVERIFY(connectionThree->authenticate(m_testToken));
    connectionThreeDisconnectSpy.wait();

    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(!connectionThree->isConnected());
    QVERIFY(!connectionThree->isAuthenticated());
    QVERIFY(connectionThree->state() == RemoteProxyConnection::StateDisconnected);

    // Clean up
    stopServer();
}

void RemoteProxyOfflineTests::sslConfigurations()
{
    // Start the server
    startServer();

    // Connect to the server (insecure disabled)
    RemoteProxyConnection *connector = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    QSignalSpy spyError(connector, &RemoteProxyConnection::errorOccured);
    QVERIFY(connector->connectServer(m_serverUrl));
    spyError.wait();
    QVERIFY(spyError.count() == 1);
    QVERIFY(connector->error() == RemoteProxyConnection::ErrorSocketError);
    QVERIFY(connector->state() == RemoteProxyConnection::StateDisconnected);

    // Connect to server (insecue enabled)
    QSignalSpy spyConnected(connector, &RemoteProxyConnection::connected);
    connect(connector, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);
    connector->connectServer(m_serverUrl);
    spyConnected.wait();

    QVERIFY(connector->isConnected());

    // Disconnect and clean up
    connector->disconnectServer();
    QVERIFY(!connector->isConnected());

    connector->deleteLater();
    stopServer();
}

void RemoteProxyOfflineTests::timeout()
{
    // Start the server
    startServer();

    // Configure result
    // Start the server
    startServer();

    // Configure result
    m_mockAuthenticator->setExpectedAuthenticationError();
    m_mockAuthenticator->setTimeoutDuration(6000);

    // Create request
    QVariantMap params;
    params.insert("uuid", "uuid");
    params.insert("name", "name");
    params.insert("token", "token");

    QVariant response = invokeApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    // Clean up
    stopServer();
}

QTEST_MAIN(RemoteProxyOfflineTests)
