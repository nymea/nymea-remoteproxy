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

#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QUuid>
#include <QObject>
#include <QVariantMap>
#include <QLoggingCategory>

#include "jsonreply.h"
#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic)

namespace remoteproxyclient {

class JsonRpcClient : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcClient(ProxyConnection *connection, QObject *parent = nullptr);

    // General
    JsonReply *callHello();

    // Tunnel proxy
    JsonReply *callRegisterServer(const QUuid &serverUuid, const QString &serverName);
    JsonReply *callRegisterClient(const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid);
    JsonReply *callDisconnectClient(quint16 socketAddress);
    JsonReply *callPing(uint timestamp);

private:
    ProxyConnection *m_connection = nullptr;

    int m_commandId = 0;
    QByteArray m_dataBuffer;

    QHash<int, JsonReply *> m_replies;

    void sendRequest(const QVariantMap &request, bool slipEnabled = false);
    void processDataPacket(const QByteArray &data);

signals:
    void tunnelEstablished(const QString clientName, const QString &clientUuid);
    void tunnelProxyClientConnected(const QString &clientName, const QUuid &clientUuid, const QString &clientPeerAddress, quint16 socketAddress);
    void tunnelProxyClientDisonnected(quint16 socketAddress);

public slots:
    void processData(const QByteArray &data);

};

}

#endif // JSONRPCCLIENT_H
