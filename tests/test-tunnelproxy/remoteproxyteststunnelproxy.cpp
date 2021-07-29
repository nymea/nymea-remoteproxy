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

QTEST_MAIN(RemoteProxyTestsTunnelProxy)
