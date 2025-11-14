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

#include "tunnelproxyclientconnection.h"
#include "server/transportclient.h"
#include "tunnelproxy/tunnelproxyserverconnection.h"

namespace remoteproxy {

TunnelProxyClientConnection::TunnelProxyClientConnection(TransportClient *transportClient, const QUuid &clientUuid, const QString &clientName, QObject *parent) :
    QObject(parent),
    m_transportClient(transportClient),
    m_clientUuid(clientUuid),
    m_clientName(clientName)
{

}

TransportClient *TunnelProxyClientConnection::transportClient() const
{
    return m_transportClient;
}

TunnelProxyServerConnection *TunnelProxyClientConnection::serverConnection() const
{
    return m_serverConnection;
}

void TunnelProxyClientConnection::setServerConnection(TunnelProxyServerConnection *serverConnection)
{
    m_serverConnection = serverConnection;
    if (m_serverConnection) {
        m_serverUuid = m_serverConnection->serverUuid();
    } else {
        m_serverUuid = QUuid();
    }

}

QUuid TunnelProxyClientConnection::clientUuid() const
{
    return m_clientUuid;
}

QString TunnelProxyClientConnection::clientName() const
{
    return m_clientName;
}

QUuid TunnelProxyClientConnection::serverUuid() const
{
    return m_serverUuid;
}

quint16 TunnelProxyClientConnection::socketAddress() const
{
    return m_socketAddress;
}

void TunnelProxyClientConnection::setSocketAddress(quint16 socketAddress)
{
    m_socketAddress = socketAddress;
}

QDebug operator<<(QDebug debug, TunnelProxyClientConnection *clientConnection)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TunnelProxyClientConnection(";
    debug.nospace() << clientConnection->clientName() << ", ";
    debug.nospace() << clientConnection->clientUuid().toString() << ", ";
    debug.nospace() << "server: " << clientConnection->serverUuid().toString() << ", ";
    debug.nospace() << clientConnection->transportClient() << ")";
    return debug;
}

}
