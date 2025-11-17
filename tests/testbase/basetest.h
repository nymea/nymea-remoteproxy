// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BASETEST_H
#define BASETEST_H

#include <QObject>
#include <QtTest>
#include <QSslKey>
#include <QObject>
#include <QWebSocket>
#include <QHostAddress>
#include <QSslCertificate>
#include <QSslConfiguration>

#include "jsonrpc/jsontypes.h"
#include "proxyconfiguration.h"

using namespace remoteproxy;

class BaseTest : public QObject
{
    Q_OBJECT
public:
    explicit BaseTest(QObject *parent = nullptr);

protected:
    ProxyConfiguration *m_configuration = nullptr;

    QUrl m_serverUrlTunnelProxyWebSocket = QUrl("wss://127.0.0.1:2212");
    QUrl m_serverUrlTunnelProxyTcp = QUrl("ssl://127.0.0.1:2213");

    QSslConfiguration m_sslConfiguration;

    QString m_testToken;

    int m_commandCounter = 0;

    void loadConfiguration(const QString &fileName);

    void resetDebugCategories();
    void addDebugCategory(const QString &debugCategory);

    void cleanUpEngine();
    void restartEngine();
    void startEngine();
    void startServer();
    void stopServer();

    QVariant invokeWebSocketTunnelProxyApiCall(const QString &method, const QVariantMap params = QVariantMap());
    QVariant injectWebSocketTunnelProxyData(const QByteArray &data);

    QVariant invokeTcpSocketTunnelProxyApiCall(const QString &method, const QVariantMap params = QVariantMap());
    QVariant injectTcpSocketTunnelProxyData(const QByteArray &data);

    QPair<QVariant, QSslSocket *> invokeTcpSocketTunnelProxyApiCallPersistant(const QString &method, const QVariantMap params = QVariantMap(), bool slipEnabled = false, QSslSocket *existingSocket = nullptr);
    QPair<QVariant, QWebSocket *> invokeWebSocketTunnelProxyApiCallPersistant(const QString &method, const QVariantMap params = QVariantMap(),  bool slipEnabled = false, QWebSocket *existingSocket = nullptr);


    bool createRemoteConnection(const QString &token, const QString &nonce, QObject *parent);

protected slots:
    void initTestCase();
    void cleanupTestCase();

public slots:
    inline void sslSocketSslErrors(const QList<QSslError> &) {
        QSslSocket *socket = static_cast<QSslSocket *>(sender());
        socket->ignoreSslErrors();
    }

    inline void sslErrors(const QList<QSslError> &) {
        QWebSocket *socket = static_cast<QWebSocket *>(sender());
        socket->ignoreSslErrors();
    }

    inline void verifyError(const QVariant &response, const QString &fieldName, const QString &error) {
        QJsonDocument jsonDoc = QJsonDocument::fromVariant(response);
        QVERIFY2(response.toMap().value("status").toString() == QString("success"),
                 QString("\nExpected status: \"success\"\nGot: %2\nFull message: %3")
                 .arg(response.toMap().value("status").toString())
                 .arg(jsonDoc.toJson().data())
                 .toLatin1().data());
        QVERIFY2(response.toMap().value("params").toMap().value(fieldName).toString() == error,
                 QString("\nExpected: %1\nGot: %2\nFull message: %3\n")
                 .arg(error)
                 .arg(response.toMap().value("params").toMap().value(fieldName).toString())
                 .arg(jsonDoc.toJson().data())
                 .toLatin1().data());
    }

    inline void verifyTunnelProxyError(const QVariant &response, TunnelProxyServer::TunnelProxyError error = TunnelProxyServer::TunnelProxyErrorNoError) {
        verifyError(response, "tunnelProxyError", JsonTypes::tunnelProxyErrorToString(error));
    }

private:
    QString m_defaultDebugCategories = "*.debug=false\ndefault.debug=true\nApplication.debug=true\n";
    QString m_currentDebugCategories;
};

#endif // BASETEST_H
