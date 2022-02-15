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

#include "remoteproxytestsproxy.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

RemoteProxyTestsProxy::RemoteProxyTestsProxy(QObject *parent) :
    BaseTest(parent)
{

}

void RemoteProxyTestsProxy::startStopServer()
{
    resetDebugCategories();
    addDebugCategory("ProxyServer.debug=true");
    addDebugCategory("Engine.debug=true");
    addDebugCategory("JsonRpc.debug=true");
    addDebugCategory("TcpSocketServer.debug=true");
    addDebugCategory("WebSocketServer.debug=true");

    startServer();
    stopServer();

    resetDebugCategories();
}

void RemoteProxyTestsProxy::dummyAuthenticator()
{
    resetDebugCategories();
    addDebugCategory("ProxyServer.debug=true");

    cleanUpEngine();

    m_configuration = new ProxyConfiguration(this);
    loadConfiguration(":/test-configuration.conf");

    m_dummyAuthenticator = new DummyAuthenticator(this);
    m_authenticator = qobject_cast<Authenticator *>(m_dummyAuthenticator);

    // Start proxy webserver
    Engine::instance()->setAuthenticator(m_dummyAuthenticator);
    QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->start(m_configuration);
    runningSpy.wait();

    QVERIFY(runningSpy.count() == 1);

    // Make sure the server is running
    QVERIFY(Engine::instance()->running());
    QVERIFY(Engine::instance()->webSocketServerProxy()->running());
    QVERIFY(Engine::instance()->proxyServer()->running());

    // Create request
    QVariantMap params;
    params.insert("uuid", QUuid::createUuid().toString());
    params.insert("name", "test");
    params.insert("token", "foobar");

    QVariant response = invokeWebSocketProxyApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    verifyAuthenticationError(response);

    cleanUpEngine();
}


void RemoteProxyTestsProxy::monitorServer()
{
    // Start the server
    startServer();

    QVERIFY(Engine::instance()->monitorServer()->running());

    // Create a tunnel
    // Configure mock authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nameConnectionOne = "Test client one";
    QUuid uuidConnectionOne = QUuid::createUuid();

    QString nameConnectionTwo = "Test client two";
    QUuid uuidConnectionTwo = QUuid::createUuid();

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(uuidConnectionOne, nameConnectionOne, this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(uuidConnectionTwo, nameConnectionTwo, this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyWebSocket));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyWebSocket));
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
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

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

    QCOMPARE(connectionOne->tunnelPartnerName(), nameConnectionTwo);
    QCOMPARE(QUuid(connectionOne->tunnelPartnerUuid()), uuidConnectionTwo);
    QCOMPARE(connectionTwo->tunnelPartnerName(), nameConnectionOne);
    QCOMPARE(QUuid(connectionTwo->tunnelPartnerUuid()), uuidConnectionOne);


    // Get monitor data
    QLocalSocket *monitor = new QLocalSocket(this);
    QSignalSpy connectedSpy(monitor, &QLocalSocket::connected);
    monitor->connectToServer(m_configuration->monitorSocketFileName());
    connectedSpy.wait(200);
    QVERIFY(connectedSpy.count() == 1);

    QSignalSpy dataSpy(monitor, &QLocalSocket::readyRead);
    dataSpy.wait();
    QVERIFY(dataSpy.count() == 1);
    QByteArray data = monitor->readAll();
    qDebug() << data;

    // TODO: verify monitor data

    QSignalSpy disconnectedSpy(monitor, &QLocalSocket::connected);
    monitor->disconnectFromServer();
    disconnectedSpy.wait(200);

    // Clean up
    monitor->deleteLater();
    connectionOne->deleteLater();
    connectionTwo->deleteLater();
    stopServer();
}

void RemoteProxyTestsProxy::configuration_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("success");

    QTest::newRow("valid configuration") << ":/test-configuration.conf" << true;
    QTest::newRow("valid configuration chain") << ":/test-configuration-chain.conf" << true;
    QTest::newRow("faulty filename") << ":/not-exisitng-test-configuration.conf" << false;
    QTest::newRow("faulty certificate") << ":/test-configuration-faulty-certificate.conf" << false;
    QTest::newRow("faulty key") << ":/test-configuration-faulty-key.conf" << false;
    QTest::newRow("faulty chain") << ":/test-configuration-faulty-chain.conf" << false;
}

void RemoteProxyTestsProxy::configuration()
{
    QFETCH(QString, fileName);
    QFETCH(bool, success);

    ProxyConfiguration configuration;
    QCOMPARE(configuration.loadConfiguration(fileName), success);
}

void RemoteProxyTestsProxy::serverPortBlocked()
{
    cleanUpEngine();

    m_configuration = new ProxyConfiguration(this);
    loadConfiguration(":/test-configuration.conf");

    m_mockAuthenticator = new MockAuthenticator(this);
    m_authenticator = qobject_cast<Authenticator *>(m_mockAuthenticator);

    // Create a dummy server which blocks the port
    QWebSocketServer dummyServer("dummy-server", QWebSocketServer::NonSecureMode);
    QVERIFY(dummyServer.listen(QHostAddress::LocalHost, m_configuration->webSocketServerProxyPort()));

    // Start proxy webserver
    QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->setAuthenticator(m_authenticator);
    Engine::instance()->start(m_configuration);
    runningSpy.wait();
    QVERIFY(runningSpy.count() == 1);

    // Make sure the server is not running
    QVERIFY(Engine::instance()->running());

    // Make sure the websocket server is not running
    QVERIFY(!Engine::instance()->webSocketServerProxy()->running());

    QSignalSpy closedSpy(&dummyServer, &QWebSocketServer::closed);
    dummyServer.close();
    closedSpy.wait();
    QVERIFY(closedSpy.count() == 1);

    // Try again
    startServer();

    // Clean up
    stopServer();

    // Do the same with the tcp server
    cleanUpEngine();

    m_configuration = new ProxyConfiguration(this);
    loadConfiguration(":/test-configuration.conf");

    m_mockAuthenticator = new MockAuthenticator(this);
    m_authenticator = qobject_cast<Authenticator *>(m_mockAuthenticator);

    // Create a dummy server which blocks the port
    QTcpServer *tcpDummyServer = new QTcpServer(this);
    QVERIFY(tcpDummyServer->listen(QHostAddress::LocalHost, m_configuration->tcpServerPort()));

    // Start proxy webserver
    QSignalSpy runningSpy2(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->setAuthenticator(m_authenticator);
    Engine::instance()->start(m_configuration);
    runningSpy2.wait();
    QVERIFY(runningSpy2.count() == 1);

    // Make sure the engine is running
    QVERIFY(Engine::instance()->running());

    // Make sure the TCP server is not running
    QVERIFY(!Engine::instance()->tcpSocketServerProxy()->running());

    tcpDummyServer->close();
    delete tcpDummyServer;

    // Try again
    startServer();

    // Make sure the TCP server is not running
    QVERIFY(Engine::instance()->webSocketServerProxy()->running());
    QVERIFY(Engine::instance()->tcpSocketServerProxy()->running());

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::websocketBinaryData()
{
    // Start the server
    startServer();

    QWebSocket *client = new QWebSocket("bad-client", QWebSocketProtocol::Version13);
    connect(client, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
    QSignalSpy spyConnection(client, SIGNAL(connected()));
    client->open(Engine::instance()->webSocketServerProxy()->serverUrl());
    spyConnection.wait();

    // Send binary data and make sure the server disconnects this socket
    QSignalSpy spyDisconnected(client, SIGNAL(disconnected()));
    client->sendBinaryMessage("trying to upload stuff...stuff...more stuff... other stuff");
    spyConnection.wait(200);
    QVERIFY(spyConnection.count() == 1);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::websocketPing()
{
    // Start the server
    startServer();

    QWebSocket *client = new QWebSocket("bad-client", QWebSocketProtocol::Version13);
    connect(client, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
    QSignalSpy spyConnection(client, SIGNAL(connected()));
    client->open(Engine::instance()->webSocketServerProxy()->serverUrl());
    spyConnection.wait();
    QVERIFY(spyConnection.count() == 1);

    QByteArray pingMessage = QByteArray("ping!");
    QSignalSpy pongSpy(client, &QWebSocket::pong);
    client->ping(pingMessage);
    pongSpy.wait();
    QVERIFY(pongSpy.count() == 1);
    qDebug() << pongSpy.at(0).at(0) << pongSpy.at(0).at(1);

    QVERIFY(pongSpy.at(0).at(1).toByteArray() == pingMessage);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::apiBasicCallsTcp_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("responseId");
    QTest::addColumn<QString>("responseStatus");

    QTest::newRow("valid call") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\"}") << 42 << "success";
    QTest::newRow("missing id") << QByteArray("{\"method\":\"RemoteProxy.Hello\"}") << -1 << "error";
    QTest::newRow("missing method") << QByteArray("{\"id\":42}") << 42 << "error";
    //QTest::newRow("invalid json") << QByteArray("{\"id\":42, \"method\":\"RemoteProx") << -1 << "error";
    QTest::newRow("invalid function") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Explode\"}") << 42 << "error";
    QTest::newRow("invalid namespace") << QByteArray("{\"id\":42, \"method\":\"ProxyRemote.Hello\"}") << 42 << "error";
    QTest::newRow("missing dot") << QByteArray("{\"id\":42, \"method\":\"RemoteProxyHello\"}") << 42 << "error";
    QTest::newRow("invalid params") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\", \"params\":{\"törööö\":\"chooo-chooo\"}}") << 42 << "error";
    QTest::newRow("invalid authentication params") << QByteArray("{\"id\":42, \"method\":\"Authentication.Authenticate\", \"params\":{\"your\":\"mamma\"}}") << 42 << "error";
}

void RemoteProxyTestsProxy::apiBasicCallsTcp()
{
    QFETCH(QByteArray, data);
    QFETCH(int, responseId);
    QFETCH(QString, responseStatus);

    // Start the server
    startServer();

    QVariant response = injectTcpSocketProxyData(data);
    QVERIFY(!response.isNull());

    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    QCOMPARE(response.toMap().value("id").toInt(), responseId);
    QCOMPARE(response.toMap().value("status").toString(), responseStatus);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::getIntrospect()
{
    // Start the server
    startServer();

    QVariantMap response;

    // WebSocket
    response = invokeWebSocketProxyApiCall("RemoteProxy.Introspect").toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("methods"));
    QVERIFY(response.value("params").toMap().contains("notifications"));
    QVERIFY(response.value("params").toMap().contains("types"));

    // Tcp
    response.clear();
    response = invokeTcpSocketProxyApiCall("RemoteProxy.Introspect").toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("methods"));
    QVERIFY(response.value("params").toMap().contains("notifications"));
    QVERIFY(response.value("params").toMap().contains("types"));

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::getHello()
{
    // Start the server
    startServer();
    QVariantMap response;

    // WebSocket
    response = invokeWebSocketProxyApiCall("RemoteProxy.Hello").toMap();
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Verify data
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("params").toMap().value("name").toString(), Engine::instance()->configuration()->serverName());
    QCOMPARE(response.value("params").toMap().value("server").toString(), QString(SERVER_NAME_STRING));
    QCOMPARE(response.value("params").toMap().value("version").toString(), QString(SERVER_VERSION_STRING));
    QCOMPARE(response.value("params").toMap().value("apiVersion").toString(), QString(API_VERSION_STRING));

    // TCP
    response.clear();
    response = invokeTcpSocketProxyApiCall("RemoteProxy.Hello").toMap();
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Verify data
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("params").toMap().value("name").toString(), Engine::instance()->configuration()->serverName());
    QCOMPARE(response.value("params").toMap().value("server").toString(), QString(SERVER_NAME_STRING));
    QCOMPARE(response.value("params").toMap().value("version").toString(), QString(SERVER_VERSION_STRING));
    QCOMPARE(response.value("params").toMap().value("apiVersion").toString(), QString(API_VERSION_STRING));


    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::apiBasicCalls_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("responseId");
    QTest::addColumn<QString>("responseStatus");

    QTest::newRow("valid call") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\"}") << 42 << "success";
    QTest::newRow("missing id") << QByteArray("{\"method\":\"RemoteProxy.Hello\"}") << -1 << "error";
    QTest::newRow("missing method") << QByteArray("{\"id\":42}") << 42 << "error";
    QTest::newRow("invalid json") << QByteArray("{\"id\":42, \"method\":\"RemoteProx}") << -1 << "error";
    QTest::newRow("invalid function") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Explode\"}") << 42 << "error";
    QTest::newRow("invalid namespace") << QByteArray("{\"id\":42, \"method\":\"ProxyRemote.Hello\"}") << 42 << "error";
    QTest::newRow("missing dot") << QByteArray("{\"id\":42, \"method\":\"RemoteProxyHello\"}") << 42 << "error";
    QTest::newRow("invalid params") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\", \"params\":{\"törööö\":\"chooo-chooo\"}}") << 42 << "error";
    QTest::newRow("invalid authentication params") << QByteArray("{\"id\":42, \"method\":\"Authentication.Authenticate\", \"params\":{\"your\":\"mamma\"}}") << 42 << "error";
}

void RemoteProxyTestsProxy::apiBasicCalls()
{
    QFETCH(QByteArray, data);
    QFETCH(int, responseId);
    QFETCH(QString, responseStatus);

    // Start the server
    startServer();

    QVariantMap response;

    // Websocket
    response = injectWebSocketProxyData(data).toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("id").toInt(), responseId);
    QCOMPARE(response.value("status").toString(), responseStatus);

    // TCP
    response.clear();
    response = injectTcpSocketProxyData(data).toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("id").toInt(), responseId);
    QCOMPARE(response.value("status").toString(), responseStatus);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::authenticate_data()
{
    QTest::addColumn<QString>("uuid");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("token");
    QTest::addColumn<QString>("nonce");

    QTest::addColumn<int>("timeout");
    QTest::addColumn<Authenticator::AuthenticationError>("expectedError");

    QTest::newRow("success") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << ""
                             << 100 << Authenticator::AuthenticationErrorNoError;

    QTest::newRow("success") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << "nonce"
                             << 100 << Authenticator::AuthenticationErrorNoError;

    QTest::newRow("success") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << "nonce"
                             << 100 << Authenticator::AuthenticationErrorAuthenticationFailed;


    QTest::newRow("failed") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << ""
                            << 100 << Authenticator::AuthenticationErrorAuthenticationFailed;

    QTest::newRow("not responding") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << ""
                                    << 200 << Authenticator::AuthenticationErrorProxyError;

    QTest::newRow("aborted") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << ""
                             << 100 << Authenticator::AuthenticationErrorAborted;

    QTest::newRow("unknown") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken << ""
                             << 100 << Authenticator::AuthenticationErrorUnknown;

}

void RemoteProxyTestsProxy::authenticate()
{
    QFETCH(QString, uuid);
    QFETCH(QString, name);
    QFETCH(QString, token);
    QFETCH(QString, nonce);
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
    if (!nonce.isEmpty()) params.insert("nonce", nonce);

    // WebSocket
    QVariantMap response;
    response = invokeWebSocketProxyApiCall("Authentication.Authenticate", params).toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    verifyAuthenticationError(response, expectedError);

    // TCP
    response = invokeTcpSocketProxyApiCall("Authentication.Authenticate", params).toMap();
    verifyAuthenticationError(response, expectedError);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::authenticateNonce()
{
    // Start the server
    startServer();

    QString nonce = "67af856c4e4071833ed01128e50b3ea5";
    QString token = "C001L0ckingT0ken";

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nameConnectionOne = "Test client one";
    QUuid uuidConnectionOne = QUuid::createUuid();

    QString nameConnectionTwo = "Test client two";
    QUuid uuidConnectionTwo = QUuid::createUuid();

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(uuidConnectionOne, nameConnectionOne, this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(uuidConnectionTwo, nameConnectionTwo, this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyWebSocket));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyWebSocket));
    connectionTwoReadySpy.wait();
    QVERIFY(connectionTwoReadySpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());

    // Authenticate one
    QSignalSpy remoteConnectionEstablishedOne(connectionOne, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionOne->authenticate(m_testToken, nonce));
    connectionOneAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(connectionOne->isConnected());
    QVERIFY(connectionOne->isAuthenticated());
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken, nonce));
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

    QCOMPARE(connectionOne->tunnelPartnerName(), nameConnectionTwo);
    QCOMPARE(QUuid(connectionOne->tunnelPartnerUuid()), uuidConnectionTwo);
    QCOMPARE(connectionTwo->tunnelPartnerName(), nameConnectionOne);
    QCOMPARE(QUuid(connectionTwo->tunnelPartnerUuid()), uuidConnectionOne);

    // Pipe data trought the tunnel
    QSignalSpy remoteConnectionDataOne(connectionOne, &RemoteProxyConnection::dataReady);
    QSignalSpy remoteConnectionDataTwo(connectionTwo, &RemoteProxyConnection::dataReady);

    connectionOne->sendData(dataOne);
    remoteConnectionDataTwo.wait(500);
    QVERIFY(remoteConnectionDataTwo.count() == 1);
    QCOMPARE(remoteConnectionDataTwo.at(0).at(0).toByteArray().trimmed(), dataOne);

    connectionTwo->sendData(dataTwo);
    remoteConnectionDataOne.wait(500);
    QVERIFY(remoteConnectionDataOne.count() == 1);
    QCOMPARE(remoteConnectionDataOne.at(0).at(0).toByteArray().trimmed(), dataTwo);

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::authenticateSendData()
{
    // Start the server
    startServer();

    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QVariantMap params;
    params.insert("uuid", "uuid");
    params.insert("name", "name");
    params.insert("token", "token");
    params.insert("nonce", "nonce");

    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", "Authentication.Authenticate");
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    // Connect socket
    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
    QSignalSpy spyConnection(socket, &QWebSocket::connected);
    socket->open(Engine::instance()->webSocketServerProxy()->serverUrl());
    spyConnection.wait();
    QVERIFY(spyConnection.count() == 1);

    // Authenticate
    QSignalSpy dataSpy(socket, &QWebSocket::textMessageReceived);
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
    dataSpy.wait();
    QVERIFY(dataSpy.count() == 1);

    // Send data again and make sure we get disconnected since sending data while waiting for the partner is forbidden
    QSignalSpy disconnectedSpy(socket, SIGNAL(disconnected()));
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
    disconnectedSpy.wait();
    QVERIFY(disconnectedSpy.count() == 1);

    socket->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::clientConnectionWebSocket()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    // Connect to the server using WebSocket (insecure disabled)
    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    connect(connection, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect to server (insecue enabled for testing)
    QSignalSpy readySpy(connection, &RemoteProxyConnection::ready);
    QVERIFY(connection->connectServer(m_serverUrlProxyWebSocket));
    readySpy.wait();
    QVERIFY(readySpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(!connection->isRemoteConnected());
    QVERIFY(connection->state() == RemoteProxyConnection::StateReady);
    QVERIFY(connection->error() == QAbstractSocket::UnknownSocketError);
    QVERIFY(connection->serverUrl() == m_serverUrlProxyWebSocket);
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
    QVERIFY(connection->state() == RemoteProxyConnection::StateAuthenticated);

    // Disconnect and clean up
    QSignalSpy spyDisconnected(connection, &RemoteProxyConnection::disconnected);
    connection->disconnectServer();
    // FIXME: check why it waits the full time here
    spyDisconnected.wait(500);

    QVERIFY(spyDisconnected.count() >= 1);
    QVERIFY(!connection->isConnected());

    connection->deleteLater();
    stopServer();
}

void RemoteProxyTestsProxy::clientConnectionTcpSocket()
{
    // Start the server
    startServer();

    // Configure mock authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    // Connect to the server using TCP (insecure disabled)
    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", RemoteProxyConnection::ConnectionTypeTcpSocket, this);
    connect(connection, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect to server (insecue enabled for testing)
    QSignalSpy readySpy(connection, &RemoteProxyConnection::ready);
    QVERIFY(connection->connectServer(m_serverUrlProxyTcp));
    readySpy.wait();
    QVERIFY(readySpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(!connection->isRemoteConnected());
    QVERIFY(connection->state() == RemoteProxyConnection::StateReady);
    QVERIFY(connection->error() == QAbstractSocket::UnknownSocketError);
    QVERIFY(connection->serverUrl() == m_serverUrlProxyTcp);
    QVERIFY(connection->connectionType() == RemoteProxyConnection::ConnectionTypeTcpSocket);
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
    QVERIFY(connection->state() == RemoteProxyConnection::StateAuthenticated);

    // Disconnect and clean up
    QSignalSpy spyDisconnected(connection, &RemoteProxyConnection::disconnected);
    connection->disconnectServer();
    // FIXME: check why it waits the full time here
    spyDisconnected.wait(500);

    QVERIFY(spyDisconnected.count() >= 1);
    QVERIFY(!connection->isConnected());

    connection->deleteLater();
    stopServer();
}

void RemoteProxyTestsProxy::remoteConnection()
{
    // Start the server
    startServer();

    // Configure mock authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nameConnectionOne = "Test client one";
    QUuid uuidConnectionOne = QUuid::createUuid();

    QString nameConnectionTwo = "Test client two";
    QUuid uuidConnectionTwo = QUuid::createUuid();

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(uuidConnectionOne, nameConnectionOne, this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(uuidConnectionTwo, nameConnectionTwo, this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyWebSocket));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyWebSocket));
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
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

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

    QCOMPARE(connectionOne->tunnelPartnerName(), nameConnectionTwo);
    QCOMPARE(QUuid(connectionOne->tunnelPartnerUuid()), uuidConnectionTwo);
    QCOMPARE(connectionTwo->tunnelPartnerName(), nameConnectionOne);
    QCOMPARE(QUuid(connectionTwo->tunnelPartnerUuid()), uuidConnectionOne);

    // Pipe data trought the tunnel
    QSignalSpy remoteConnectionDataOne(connectionOne, &RemoteProxyConnection::dataReady);
    QSignalSpy remoteConnectionDataTwo(connectionTwo, &RemoteProxyConnection::dataReady);

    connectionOne->sendData(dataOne);
    remoteConnectionDataTwo.wait(500);
    QVERIFY(remoteConnectionDataTwo.count() == 1);
    QCOMPARE(remoteConnectionDataTwo.at(0).at(0).toByteArray().trimmed(), dataOne);

    connectionTwo->sendData(dataTwo);
    remoteConnectionDataOne.wait(500);
    QVERIFY(remoteConnectionDataOne.count() == 1);
    QCOMPARE(remoteConnectionDataOne.at(0).at(0).toByteArray().trimmed(), dataTwo);

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::multipleRemoteConnection()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setExpectedAuthenticationError();
    m_mockAuthenticator->setTimeoutDuration(1000);
    m_configuration->setAuthenticationTimeout(2000);
    m_configuration->setJsonRpcTimeout(5000);
    m_configuration->setInactiveTimeout(5000);

    // Create multiple tunnels with one token, but different nonces for each connection
    QObject *parent = new QObject(this);
    for (int i = 0; i < 5; i++) {
        qDebug() << "============== Create remote connection" << i;
        bool connectionResult = createRemoteConnection(m_testToken, QUuid::createUuid().toString(), parent);
        if (!connectionResult) {
            qWarning() << "Test failed because could not create remote connection";
            delete parent;
        }
        QVERIFY(connectionResult);
    }

    delete parent;

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::trippleConnection()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nonce = QUuid::createUuid().toString();

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(QUuid::createUuid(), "Test client two", this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionThree = new RemoteProxyConnection(QUuid::createUuid(), "Token thief ^v^", this);
    connect(connectionThree, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyWebSocket));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyWebSocket));
    connectionTwoReadySpy.wait();
    QVERIFY(connectionTwoReadySpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());

    // Authenticate one
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionOne->authenticate(m_testToken, nonce));
    connectionOneAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(connectionOne->isConnected());
    QVERIFY(connectionOne->isAuthenticated());
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

    QSignalSpy remoteConnectionEstablishedOne(connectionOne, &RemoteProxyConnection::remoteConnectionEstablished);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken, nonce));
    connectionTwoAuthenticatedSpy.wait();
    QVERIFY(connectionTwoAuthenticatedSpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());
    QVERIFY(connectionTwo->isAuthenticated());

    // Wait for both to be connected
    remoteConnectionEstablishedOne.wait(500);

    // Now connect a third connection and make sure the client will be closed

    // Connect three
    QSignalSpy connectionThreeReadySpy(connectionThree, &RemoteProxyConnection::ready);
    QSignalSpy connectionThreeDisconnectedSpy(connectionThree, &RemoteProxyConnection::disconnected);
    QVERIFY(connectionThree->connectServer(m_serverUrlProxyWebSocket));
    connectionThreeReadySpy.wait();
    QVERIFY(connectionThreeReadySpy.count() == 1);
    QVERIFY(connectionThree->isConnected());

    // Authenticate three
    QSignalSpy connectionThreeAuthenticatedSpy(connectionThree, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionThree->authenticate(m_testToken, nonce));
    connectionThreeAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);

    connectionThreeDisconnectedSpy.wait(200);
    QVERIFY(connectionThreeDisconnectedSpy.count() >= 1);

    // Make sure the one and two are still connected
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateRemoteConnected);
    QVERIFY(connectionTwo->state() == RemoteProxyConnection::StateRemoteConnected);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::duplicateUuid()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_mockAuthenticator->setTimeoutDuration(10);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QUuid connectionUuid = QUuid::createUuid();

    QString nameConnectionOne = "Test client one";
    QString nameConnectionTwo = "Test client two";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(connectionUuid, nameConnectionOne, this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(connectionUuid, nameConnectionTwo, this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyWebSocket));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyWebSocket));
    connectionTwoReadySpy.wait();
    QVERIFY(connectionTwoReadySpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());

    // Authenticate one
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionOne->authenticate(m_testToken));
    connectionOneAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(connectionOne->isConnected());
    QVERIFY(connectionOne->isAuthenticated());
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

    // Authenticate two
    QSignalSpy disconnectSpyOne(connectionOne, &RemoteProxyConnection::disconnected);
    QSignalSpy disconnectSpyTwo(connectionTwo, &RemoteProxyConnection::disconnected);

    QVERIFY(connectionTwo->authenticate(m_testToken));
    disconnectSpyOne.wait(200);
    QVERIFY(disconnectSpyOne.count() >= 1);

    disconnectSpyTwo.wait(200);
    QVERIFY(disconnectSpyTwo.count() >= 1);

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::sslConfigurations()
{
    // Start the server
    startServer();

    // Connect to the server (insecure disabled)
    RemoteProxyConnection *connector = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    QSignalSpy spyError(connector, &RemoteProxyConnection::errorOccurred);
    QVERIFY(connector->connectServer(m_serverUrlProxyWebSocket));
    spyError.wait();
    QVERIFY(spyError.count() == 1);
    qDebug() << connector->error();
    QCOMPARE(connector->error(), QAbstractSocket::SslHandshakeFailedError);
    QVERIFY(connector->state() == RemoteProxyConnection::StateDisconnected);

    // Connect to server (insecue enabled)
    QSignalSpy spyConnected(connector, &RemoteProxyConnection::connected);
    connect(connector, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);
    connector->connectServer(m_serverUrlProxyWebSocket);
    spyConnected.wait();

    QVERIFY(connector->isConnected());

    // Disconnect and clean up
    connector->disconnectServer();
    QVERIFY(!connector->isConnected());

    connector->deleteLater();
    stopServer();
}

void RemoteProxyTestsProxy::jsonRpcTimeout()
{
    // Start the server
    startServer();

    m_configuration->setAuthenticationTimeout(3000);
    m_configuration->setJsonRpcTimeout(1000);
    m_configuration->setInactiveTimeout(2000);

    // Configure result (authentication takes longer than json rpc timeout
    m_mockAuthenticator->setExpectedAuthenticationError();
    m_mockAuthenticator->setTimeoutDuration(4000);
    m_configuration->setAuthenticationTimeout(4000);
    m_configuration->setJsonRpcTimeout(1000);
    m_configuration->setInactiveTimeout(2000);

    // Create request
    QVariantMap params;
    params.insert("uuid", "uuid");
    params.insert("name", "name");
    params.insert("token", "token");

    QVariant response = invokeWebSocketProxyApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    QVERIFY(response.toMap().value("status").toString() == "error");

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::inactiveTimeout()
{
    // Start the server
    startServer();

    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Sleepy test client", this);
    connect(connection, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionReadySpy(connection, &RemoteProxyConnection::ready);
    QSignalSpy connectionDisconnectedSpy(connection, &RemoteProxyConnection::disconnected);
    QVERIFY(connection->connectServer(m_serverUrlProxyWebSocket));
    connectionReadySpy.wait();
    QVERIFY(connectionReadySpy.count() == 1);

    // Now wait for disconnected
    connectionDisconnectedSpy.wait();
    QVERIFY(connectionDisconnectedSpy.count() >= 1);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::authenticationReplyTimeout()
{
    // Start the server
    startServer();

    // Configure result (authentication takes longer than json rpc timeout
    m_mockAuthenticator->setExpectedAuthenticationError();
    m_mockAuthenticator->setTimeoutDuration(2000);

    m_configuration->setAuthenticationTimeout(500);
    m_configuration->setJsonRpcTimeout(1000);
    m_configuration->setInactiveTimeout(1000);

    // Create request
    QVariantMap params;
    params.insert("uuid", QUuid::createUuid().toString());
    params.insert("name", "Sleepy test client");
    params.insert("token", "sleepy token zzzZZZ");

    QVariant response = invokeWebSocketProxyApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    verifyAuthenticationError(response, Authenticator::AuthenticationErrorTimeout);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::authenticationReplyConnection()
{
    // Start the server
    startServer();

    // Configure result (authentication takes longer than json rpc timeout
    m_mockAuthenticator->setExpectedAuthenticationError();
    m_mockAuthenticator->setTimeoutDuration(1000);

    m_configuration->setAuthenticationTimeout(500);
    m_configuration->setJsonRpcTimeout(1000);
    m_configuration->setInactiveTimeout(1000);

    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Sleepy test client", this);
    connect(connection, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionReadySpy(connection, &RemoteProxyConnection::ready);
    QVERIFY(connection->connectServer(m_serverUrlProxyWebSocket));
    connectionReadySpy.wait();
    QVERIFY(connectionReadySpy.count() == 1);

    QSignalSpy connectionErrorSpy(connection, &RemoteProxyConnection::errorOccurred);
    connection->authenticate("blub");
    connectionErrorSpy.wait();
    QVERIFY(connectionErrorSpy.count() == 1);
    QVERIFY(connectionErrorSpy.at(0).at(0).toInt() == static_cast<int>(QAbstractSocket::ProxyConnectionRefusedError));
    QVERIFY(connection->error() == QAbstractSocket::ProxyConnectionRefusedError);

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::tcpRemoteConnection()
{
    // Start the server
    startServer();

    // Configure mock authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nameConnectionOne = "Test client one";
    QUuid uuidConnectionOne = QUuid::createUuid();

    QString nameConnectionTwo = "Test client two";
    QUuid uuidConnectionTwo = QUuid::createUuid();

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(uuidConnectionOne, nameConnectionOne, RemoteProxyConnection::ConnectionTypeTcpSocket, this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(uuidConnectionTwo, nameConnectionTwo, RemoteProxyConnection::ConnectionTypeTcpSocket, this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyTcp));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyTcp));
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
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken));
    connectionTwoAuthenticatedSpy.wait();
    qDebug() << connectionTwoAuthenticatedSpy.count();
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

    QCOMPARE(connectionOne->tunnelPartnerName(), nameConnectionTwo);
    QCOMPARE(QUuid(connectionOne->tunnelPartnerUuid()), uuidConnectionTwo);
    QCOMPARE(connectionTwo->tunnelPartnerName(), nameConnectionOne);
    QCOMPARE(QUuid(connectionTwo->tunnelPartnerUuid()), uuidConnectionOne);

    // Pipe data trought the tunnel
    QSignalSpy remoteConnectionDataOne(connectionOne, &RemoteProxyConnection::dataReady);
    QSignalSpy remoteConnectionDataTwo(connectionTwo, &RemoteProxyConnection::dataReady);

    connectionOne->sendData(dataOne);
    remoteConnectionDataTwo.wait(500);
    QVERIFY(remoteConnectionDataTwo.count() == 1);
    QCOMPARE(remoteConnectionDataTwo.at(0).at(0).toByteArray().trimmed(), dataOne);

    connectionTwo->sendData(dataTwo);
    remoteConnectionDataOne.wait(500);
    QVERIFY(remoteConnectionDataOne.count() == 1);
    QCOMPARE(remoteConnectionDataOne.at(0).at(0).toByteArray().trimmed(), dataTwo);

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyTestsProxy::tcpWebsocketRemoteConnection()
{
    // Start the server
    startServer();

    // Configure mock authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nameConnectionOne = "Test client one";
    QUuid uuidConnectionOne = QUuid::createUuid();

    QString nameConnectionTwo = "Test client two";
    QUuid uuidConnectionTwo = QUuid::createUuid();

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(uuidConnectionOne, nameConnectionOne, RemoteProxyConnection::ConnectionTypeWebSocket, this);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(uuidConnectionTwo, nameConnectionTwo, RemoteProxyConnection::ConnectionTypeTcpSocket, this);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(m_serverUrlProxyWebSocket));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(m_serverUrlProxyTcp));
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
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateAuthenticated);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken));
    connectionTwoAuthenticatedSpy.wait();
    qDebug() << connectionTwoAuthenticatedSpy.count();
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

    QCOMPARE(connectionOne->tunnelPartnerName(), nameConnectionTwo);
    QCOMPARE(QUuid(connectionOne->tunnelPartnerUuid()), uuidConnectionTwo);
    QCOMPARE(connectionTwo->tunnelPartnerName(), nameConnectionOne);
    QCOMPARE(QUuid(connectionTwo->tunnelPartnerUuid()), uuidConnectionOne);

    // Pipe data trought the tunnel
    QSignalSpy remoteConnectionDataOne(connectionOne, &RemoteProxyConnection::dataReady);
    QSignalSpy remoteConnectionDataTwo(connectionTwo, &RemoteProxyConnection::dataReady);

    connectionOne->sendData(dataOne);
    remoteConnectionDataTwo.wait(500);
    QVERIFY(remoteConnectionDataTwo.count() == 1);
    QCOMPARE(remoteConnectionDataTwo.at(0).at(0).toByteArray().trimmed(), dataOne);

    connectionTwo->sendData(dataTwo);
    remoteConnectionDataOne.wait(500);
    QVERIFY(remoteConnectionDataOne.count() == 1);
    QCOMPARE(remoteConnectionDataOne.at(0).at(0).toByteArray().trimmed(), dataTwo);

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

QTEST_MAIN(RemoteProxyTestsProxy)
