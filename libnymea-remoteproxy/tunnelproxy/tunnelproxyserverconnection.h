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

#ifndef TUNNELPROXYSERVERCONNECTION_H
#define TUNNELPROXYSERVERCONNECTION_H

#include <QUuid>
#include <QObject>
#include <QDebug>

namespace remoteproxy {

class TransportClient;
class TunnelProxyClientConnection;

class TunnelProxyServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit TunnelProxyServerConnection(TransportClient *transportClient, const QUuid &serverUuid, const QString &serverName, QObject *parent = nullptr);

    TransportClient *transportClient() const;

    QUuid serverUuid() const;
    QString serverName() const;

    QList<TunnelProxyClientConnection *> clientConnections() const;

    void registerClientConnection(TunnelProxyClientConnection *clientConnection);
    void unregisterClientConnection(TunnelProxyClientConnection *clientConnection);

    TunnelProxyClientConnection *getClientConnection(quint16 socketAddress);

private:
    TransportClient *m_transportClient = nullptr;
    QUuid m_serverUuid;
    QString m_serverName;
    quint16 m_connectionLimit = 100;

    quint16 m_currentAddressCounter = 0;

    QHash<QUuid, TunnelProxyClientConnection *> m_clientConnections;
    QHash<quint16, TunnelProxyClientConnection *> m_clientConnectionsAddresses;

    quint64 m_lastPingTimestamp = 0;

    quint16 getFreeAddress();

};

QDebug operator<<(QDebug debug, TunnelProxyServerConnection *serverConnection);

}

#endif // TUNNELPROXYSERVERCONNECTION_H
