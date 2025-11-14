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

#ifndef TUNNELPROXYSERVER_H
#define TUNNELPROXYSERVER_H

#include <QObject>

#include "server/jsonrpcserver.h"
#include "server/transportinterface.h"
#include "tunnelproxyclient.h"

namespace remoteproxy {

class TunnelProxyServerConnection;
class TunnelProxyClientConnection;

class TunnelProxyServer : public QObject
{
    Q_OBJECT
public:
    enum TunnelProxyError {
        TunnelProxyErrorNoError,
        TunnelProxyErrorInvalidUuid,
        TunnelProxyErrorInternalServerError,
        TunnelProxyErrorServerNotFound,
        TunnelProxyErrorForbiddenCall,
        TunnelProxyErrorAlreadyRegistered,
        TunnelProxyErrorNotRegistered,
        TunnelProxyErrorUnknownSocketAddress
    };
    Q_ENUM(TunnelProxyError)

    explicit TunnelProxyServer(QObject *parent = nullptr);
    ~TunnelProxyServer();

    bool running() const;
    void setRunning(bool running);

    void registerTransportInterface(TransportInterface *interface);

    TunnelProxyServer::TunnelProxyError registerServer(const QUuid &clientId, const QUuid &serverUuid, const QString &serverName);
    TunnelProxyServer::TunnelProxyError registerClient(const QUuid &clientId, const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid);
    TunnelProxyServer::TunnelProxyError disconnectClient(const QUuid &clientId, quint16 socketAddress);

    QVariantMap currentStatistics(bool printAll = false);

public slots:
    void startServer();
    void stopServer();

    void tick();

signals:
    void runningChanged(bool running);

private slots:
    void onClientConnected(const QUuid &clientId, const QHostAddress &address);
    void onClientDisconnected(const QUuid &clientId);
    void onClientDataAvailable(const QUuid &clientId, const QByteArray &data);

private:
    JsonRpcServer *m_jsonRpcServer = nullptr;
    QList<TransportInterface *> m_transportInterfaces;

    bool m_running = false;

    QHash<QUuid, TunnelProxyClient *> m_proxyClients; // clientId, object

    // Server connections
    QHash<QUuid, TunnelProxyServerConnection *> m_tunnelProxyServerConnections; // server uuid, object
    QHash<QUuid, TunnelProxyClientConnection *> m_tunnelProxyClientConnections; // client uuid, object

    // Statistic measurments
    int m_troughput = 0;
    int m_troughputCounter = 0;
};

}

#endif // TUNNELPROXYSERVER_H
