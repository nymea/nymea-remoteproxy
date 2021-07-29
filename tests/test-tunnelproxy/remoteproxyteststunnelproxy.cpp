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
    QVariantMap response;

    // WebSocket
    response = invokeWebSocketTunnelProxyApiCall("RemoteProxy.Hello").toMap();
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

void RemoteProxyTestsTunnelProxy::registerServerDuplicated()
{
    // Start the server
    startServer();

    resetDebugCategories();
    addDebugCategory("TunnelProxyServer.debug=true");

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


QTEST_MAIN(RemoteProxyTestsTunnelProxy)
