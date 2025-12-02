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

#ifndef TUNNELPROXYCLIENTCONNECTION_H
#define TUNNELPROXYCLIENTCONNECTION_H

#include <QUuid>
#include <QObject>
#include <QDebug>

namespace remoteproxy {

class TransportClient;
class TunnelProxyServerConnection;

class TunnelProxyClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit TunnelProxyClientConnection(TransportClient *transportClient, const QUuid &clientUuid, const QString &clientName, QObject *parent = nullptr);

    TransportClient *transportClient() const;

    TunnelProxyServerConnection *serverConnection() const;
    void setServerConnection(TunnelProxyServerConnection *serverConnection);

    QUuid clientUuid() const;
    QString clientName() const;
    QUuid serverUuid() const;

    quint16 socketAddress() const;
    void setSocketAddress(quint16 socketAddress);

private:
    TransportClient *m_transportClient = nullptr;
    TunnelProxyServerConnection *m_serverConnection = nullptr;

    QUuid m_clientUuid;
    QString m_clientName;
    QUuid m_serverUuid;
    quint16 m_socketAddress = 0xFFFF;
};

QDebug operator<<(QDebug debug, TunnelProxyClientConnection *clientConnection);

}

#endif // TUNNELPROXYCLIENTCONNECTION_H
