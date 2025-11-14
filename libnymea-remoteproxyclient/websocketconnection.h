// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef WEBSOCKETCONNECTOR_H
#define WEBSOCKETCONNECTOR_H

#include <QDebug>
#include <QObject>
#include <QWebSocket>
#include <QLoggingCategory>

#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientWebSocket)

namespace remoteproxyclient {

class WebSocketConnection : public ProxyConnection
{
    Q_OBJECT
public:
    explicit WebSocketConnection(QObject *parent = nullptr);
    ~WebSocketConnection() override;

    void sendData(const QByteArray &data) override;

    void ignoreSslErrors() override;
    void ignoreSslErrors(const QList<QSslError> &errors) override;

private:
    QWebSocket *m_webSocket = nullptr;

private slots:
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onTextMessageReceived(const QString &message);

public slots:
    void connectServer(const QUrl &serverUrl) override;
    void disconnectServer() override;

};

}

#endif // WEBSOCKETCONNECTOR_H
