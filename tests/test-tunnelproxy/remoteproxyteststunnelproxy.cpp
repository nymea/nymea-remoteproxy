/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2021, nymea GmbH
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

#include "remoteproxyteststunnelproxy.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"

// Client
#include "tunnelproxy/tunnelproxysocketserver.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

RemoteProxyTestsTunnelProxy::RemoteProxyTestsTunnelProxy(QObject *parent) :
    BaseTest(parent)
{

}

void RemoteProxyTestsTunnelProxy::startStopServer()
{
    startServer();
    stopServer();
}

void RemoteProxyTestsTunnelProxy::apiBasicCallsTcp_data()
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

void RemoteProxyTestsTunnelProxy::apiBasicCallsTcp()
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

void RemoteProxyTestsTunnelProxy::getIntrospect()
{
    // Start the server
    startServer();

    QVariantMap response;

    // WebSocket
    response = invokeWebSocketTunnelProxyApiCall("RemoteProxy.Introspect").toMap();
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("methods"));
    QVERIFY(response.value("params").toMap().contains("notifications"));
    QVERIFY(response.value("params").toMap().contains("types"));

    // Tcp
    response.clear();
    response = invokeTcpSocketTunnelProxyApiCall("RemoteProxy.Introspect").toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("methods"));
    QVERIFY(response.value("params").toMap().contains("notifications"));
    QVERIFY(response.value("params").toMap().contains("types"));

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::getHello()
{
    // Start the server
    startServer();

    // WebSocket
    QVariantMap response = invokeWebSocketTunnelProxyApiCall("RemoteProxy.Hello").toMap();
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Verify data
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("params").toMap().value("name").toString(), Engine::instance()->configuration()->serverName());
    QCOMPARE(response.value("params").toMap().value("server").toString(), QString(SERVER_NAME_STRING));
    QCOMPARE(response.value("params").toMap().value("version").toString(), QString(SERVER_VERSION_STRING));
    QCOMPARE(response.value("params").toMap().value("apiVersion").toString(), QString(API_VERSION_STRING));

    // TCP
    response.clear();
    response = invokeTcpSocketTunnelProxyApiCall("RemoteProxy.Hello").toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Verify data
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("params").toMap().value("name").toString(), Engine::instance()->configuration()->serverName());
    QCOMPARE(response.value("params").toMap().value("server").toString(), QString(SERVER_NAME_STRING));
    QCOMPARE(response.value("params").toMap().value("version").toString(), QString(SERVER_VERSION_STRING));
    QCOMPARE(response.value("params").toMap().value("apiVersion").toString(), QString(API_VERSION_STRING));

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::apiBasicCalls_data()
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

void RemoteProxyTestsTunnelProxy::apiBasicCalls()
{
    QFETCH(QByteArray, data);
    QFETCH(int, responseId);
    QFETCH(QString, responseStatus);

    // Start the server
    startServer();

    QVariantMap response;

    // Websocket
    response = injectWebSocketTunnelProxyData(data).toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("id").toInt(), responseId);
    QCOMPARE(response.value("status").toString(), responseStatus);

    // TCP
    response.clear();
    response = injectTcpSocketTunnelProxyData(data).toMap();
    //qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    QVERIFY(!response.isEmpty());
    QCOMPARE(response.value("id").toInt(), responseId);
    QCOMPARE(response.value("status").toString(), responseStatus);

    // Clean up
    stopServer();
}


void RemoteProxyTestsTunnelProxy::registerServer_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("uuid");
    QTest::addColumn<TunnelProxyServer::TunnelProxyError>("expectedError");

    QTest::newRow("valid call") << "valid server" << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorNoError;
    QTest::newRow("valid uuid with brackets") << "valid server" << "{00323a95-d1ab-4752-88d4-dbc8b9015b0f}" << TunnelProxyServer::TunnelProxyErrorNoError;
    QTest::newRow("valid uuid without brackets") << "valid server" << "00323a95-d1ab-4752-88d4-dbc8b9015b0f" << TunnelProxyServer::TunnelProxyErrorNoError;
    QTest::newRow("invalid null uuid") << "valid server" << QUuid().toString() << TunnelProxyServer::TunnelProxyErrorInvalidUuid;
    QTest::newRow("invalid null uuid") << "valid server" << "foo" << TunnelProxyServer::TunnelProxyErrorInvalidUuid;
}


void RemoteProxyTestsTunnelProxy::registerServer()
{
    QFETCH(QString, name);
    QFETCH(QString, uuid);
    QFETCH(TunnelProxyServer::TunnelProxyError, expectedError);

    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");

    // Register a new server
    QVariantMap params;
    params.insert("serverName", name);
    params.insert("serverUuid", uuid);

    // Websocket
    QVariantMap response = invokeWebSocketTunnelProxyApiCall("TunnelProxy.RegisterServer", params).toMap();

    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, expectedError);

    // TCP Socket
    response = invokeTcpSocketTunnelProxyApiCall("TunnelProxy.RegisterServer", params).toMap();
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, expectedError);

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::registerClient_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("uuid");
    QTest::addColumn<TunnelProxyServer::TunnelProxyError>("expectedError");
    QTest::addColumn<bool>("serverExists");
    QTest::addColumn<QString>("serverUuid");
    QTest::addColumn<TunnelProxyServer::TunnelProxyError>("expectedServerError");

    QTest::newRow("valid client: valid server") << "Friendly client" << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorNoError << true << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorNoError;
    QTest::newRow("valid client: no server") << "Friendly client" << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorServerNotFound << false << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorNoError;
    QTest::newRow("valid client: invalid server uuid") << "Friendly client" << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorInvalidUuid << true << QUuid().toString() << TunnelProxyServer::TunnelProxyErrorInvalidUuid;
    QTest::newRow("invalid client uuid: valid server uuid") << "Friendly client" << QUuid().toString() << TunnelProxyServer::TunnelProxyErrorInvalidUuid << true << QUuid::createUuid().toString() << TunnelProxyServer::TunnelProxyErrorNoError;
    QTest::newRow("invalid client uuid: invalid server uuid") << "Friendly client" << "hello again" << TunnelProxyServer::TunnelProxyErrorInvalidUuid << true << "it's me" << TunnelProxyServer::TunnelProxyErrorInvalidUuid;
}

void RemoteProxyTestsTunnelProxy::registerClient()
{
    QFETCH(QString, name);
    QFETCH(QString, uuid);
    QFETCH(TunnelProxyServer::TunnelProxyError, expectedError);
    QFETCH(bool, serverExists);
    QFETCH(QString, serverUuid);
    QFETCH(TunnelProxyServer::TunnelProxyError, expectedServerError);

    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");

    QSslSocket *socket = nullptr;
    if (serverExists) {
        QVariantMap serverParams;
        serverParams.insert("serverName", "dummy server");
        serverParams.insert("serverUuid", serverUuid);

        // TCP socket
        QPair<QVariant, QSslSocket *> result = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", serverParams);
        QVariantMap response = result.first.toMap();
        socket = result.second;

        QVERIFY(!response.isEmpty());
        QVERIFY(response.value("status").toString() == "success");
        QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
        verifyTunnelProxyError(response, expectedServerError);
    }

    // Register a new client
    QVariantMap params;
    params.insert("clientName", name);
    params.insert("clientUuid", uuid);
    params.insert("serverUuid", serverUuid);

    // Websocket
    QVariantMap response = invokeWebSocketTunnelProxyApiCall("TunnelProxy.RegisterClient", params).toMap();
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, expectedError);

    QTest::qWait(100);

    // TCP Socket
    response = invokeTcpSocketTunnelProxyApiCall("TunnelProxy.RegisterClient", params).toMap();
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, expectedError);

    QTest::qWait(100);

    if (socket) {
        // Close the tcp socket
        socket->close();
        delete socket;
    }

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::registerServerDuplicated()
{
    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");
    addDebugCategory("JsonRpcTraffic.debug=true");

    // Register a new server
    QString serverName = "tunnel proxy server awesome nymea installation";
    QUuid serverUuid = QUuid::createUuid();

    QVariantMap params;
    params.insert("serverName", serverName);
    params.insert("serverUuid", serverUuid.toString());

    // TCP socket
    QPair<QVariant, QSslSocket *> result = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", params);
    QVariantMap response = result.first.toMap();
    QSslSocket *socket = result.second;

    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response);

    // Try to register again with the same uuid on the same socket
    result = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", params, socket);
    response = result.first.toMap();
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, TunnelProxyServer::TunnelProxyErrorAlreadyRegistered);

    // Try to register an invalid server uuid
    QVariantMap paramsInvalidUuid;
    paramsInvalidUuid.insert("serverName", serverName);
    paramsInvalidUuid.insert("serverUuid", QUuid().toString());
    result = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", paramsInvalidUuid, socket);
    response = result.first.toMap();
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, TunnelProxyServer::TunnelProxyErrorInvalidUuid);

    // Close the tcp socket
    socket->close();
    delete socket;

    QTest::qWait(100);

    // WebSocket
    // Try to register from a websocket with the same uuid
    QPair<QVariant, QWebSocket *> resultWebSocket = invokeWebSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", params);
    response = resultWebSocket.first.toMap();
    QWebSocket *webSocket = resultWebSocket.second;
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response);

    // Try to register again with the same uuid on the same socket
    resultWebSocket = invokeWebSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", params, webSocket);
    response = resultWebSocket.first.toMap();
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, TunnelProxyServer::TunnelProxyErrorAlreadyRegistered);

    // Try to register an invalid server uuid
    resultWebSocket = invokeWebSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", paramsInvalidUuid, webSocket);
    response = resultWebSocket.first.toMap();
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, TunnelProxyServer::TunnelProxyErrorInvalidUuid);

    webSocket->close();
    delete webSocket;

    QTest::qWait(100);

    resetDebugCategories();

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::registerClientDuplicated()
{
    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");
    addDebugCategory("JsonRpcTraffic.debug=true");


    // Create the server and keep it up
    QString serverName = "creative server name";
    QUuid serverUuid = QUuid::createUuid();
    QVariantMap serverParams;
    serverParams.insert("serverName", serverName);
    serverParams.insert("serverUuid", serverUuid.toString());

    // TCP socket
    QPair<QVariant, QSslSocket *> serverResult = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", serverParams);
    QVariantMap response = serverResult.first.toMap();
    QSslSocket *serverSocket = serverResult.second;

    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response);

    // Connect a client TCP
    QString clientName = "creative server name";
    QUuid clientUuid = QUuid::createUuid();
    QVariantMap clientParams;
    clientParams.insert("clientName", serverName);
    clientParams.insert("clientUuid", clientUuid.toString());
    clientParams.insert("serverUuid", serverUuid.toString());

    QPair<QVariant, QSslSocket *> clientResult = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterClient", clientParams);
    response = clientResult.first.toMap();
    QSslSocket *clientSocketTcp = clientResult.second;
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response);

    // Connect another client WebSocket
    QPair<QVariant, QSslSocket *> clientResultWS = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterClient", clientParams);
    response = clientResultWS.first.toMap();
    QSslSocket *clientSocketWs = clientResultWS.second;
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, TunnelProxyServer::TunnelProxyErrorAlreadyRegistered);


    // CleanUp
    if (clientSocketTcp) {
        // Close the tcp socket
        clientSocketTcp->close();
        delete clientSocketTcp;
    }

    if (clientSocketWs) {
        // Close the tcp socket
        clientSocketWs->close();
        delete clientSocketWs;
    }

    if (serverSocket) {
        // Close the tcp socket
        serverSocket->close();
        delete serverSocket;
    }

    resetDebugCategories();

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::crossRegisterServerClient()
{
    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");
    addDebugCategory("JsonRpcTraffic.debug=true");

    // Create the server and keep it up
    QString serverName = "creative server name";
    QUuid serverUuid = QUuid::createUuid();
    QVariantMap serverParams;
    serverParams.insert("serverName", serverName);
    serverParams.insert("serverUuid", serverUuid.toString());

    // TCP socket
    QPair<QVariant, QSslSocket *> serverResult = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterServer", serverParams);
    QVariantMap response = serverResult.first.toMap();
    QSslSocket *serverSocket = serverResult.second;
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response);


    // Now try to register as client
    QVariantMap clientParams;
    clientParams.insert("clientName", "creative client name");
    clientParams.insert("clientUuid", QUuid::createUuid().toString());
    clientParams.insert("serverUuid", serverUuid.toString());

    QPair<QVariant, QSslSocket *> result = invokeTcpSocketTunnelProxyApiCallPersistant("TunnelProxy.RegisterClient", clientParams, serverSocket);
    response = result.first.toMap();
    QVERIFY(!response.isEmpty());
    QVERIFY(response.value("status").toString() == "success");
    QVERIFY(response.value("params").toMap().contains("tunnelProxyError"));
    verifyTunnelProxyError(response, TunnelProxyServer::TunnelProxyErrorAlreadyRegistered);

    serverSocket->close();
    delete serverSocket;

    resetDebugCategories();

    // Clean up
    stopServer();
}

void RemoteProxyTestsTunnelProxy::testTunnelProxyServer()
{
    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");
    addDebugCategory("JsonRpcTraffic.debug=true");
    addDebugCategory("TunnelProxySocketServer.debug=true");

    // Tunnel proxy socket server
    QString serverName = "SuperDuper server name";
    QUuid serverUuid = QUuid::createUuid();

    TunnelProxySocketServer *tunnelProxyServer = new TunnelProxySocketServer(serverUuid, serverName, this);
    connect(tunnelProxyServer, &TunnelProxySocketServer::sslErrors, this, [=](const QList<QSslError> &errors){
        tunnelProxyServer->ignoreSslErrors(errors);
    });

    tunnelProxyServer->startServer(m_serverUrlTunnelProxyTcp);

    QSignalSpy serverRunningSpy(tunnelProxyServer, &TunnelProxySocketServer::runningChanged);
    serverRunningSpy.wait();
    QVERIFY(serverRunningSpy.count() == 1);
    QList<QVariant> arguments = serverRunningSpy.takeFirst();
    QVERIFY(arguments.at(0).toBool() == true);
    QVERIFY(tunnelProxyServer->running());


    // Tunnel proxy client connection



    resetDebugCategories();

    // Clean up
    stopServer();
}


QTEST_MAIN(RemoteProxyTestsTunnelProxy)