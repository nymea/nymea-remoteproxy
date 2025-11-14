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

#include "tunnelproxyserverconnection.h"
#include "server/transportclient.h"
#include "tunnelproxyclientconnection.h"

namespace remoteproxy {

TunnelProxyServerConnection::TunnelProxyServerConnection(TransportClient *transportClient, const QUuid &serverUuid, const QString &serverName, QObject *parent) :
    QObject(parent),
    m_transportClient(transportClient),
    m_serverUuid(serverUuid),
    m_serverName(serverName)
{

}

TransportClient *TunnelProxyServerConnection::transportClient() const
{
    return m_transportClient;
}

QUuid TunnelProxyServerConnection::serverUuid() const
{
    return m_serverUuid;
}

QString TunnelProxyServerConnection::serverName() const
{
    return m_serverName;
}

QList<TunnelProxyClientConnection *> TunnelProxyServerConnection::clientConnections() const
{
    return m_clientConnections.values();
}

void TunnelProxyServerConnection::registerClientConnection(TunnelProxyClientConnection *clientConnection)
{
    quint16 socketAddress = getFreeAddress();
    clientConnection->setSocketAddress(socketAddress);
    clientConnection->setServerConnection(this);
    m_clientConnectionsAddresses.insert(socketAddress, clientConnection);
    m_clientConnections.insert(clientConnection->clientUuid(), clientConnection);
}

void TunnelProxyServerConnection::unregisterClientConnection(TunnelProxyClientConnection *clientConnection)
{
    m_clientConnections.remove(clientConnection->clientUuid());
    m_clientConnectionsAddresses.remove(clientConnection->socketAddress());
    clientConnection->setSocketAddress(0xFFFF);
    clientConnection->setServerConnection(nullptr);
}

TunnelProxyClientConnection *TunnelProxyServerConnection::getClientConnection(quint16 socketAddress)
{
    return m_clientConnectionsAddresses.value(socketAddress);
}

quint16 TunnelProxyServerConnection::getFreeAddress()
{
    m_currentAddressCounter += 1;
    quint16 address = m_currentAddressCounter;
    for (int i = 0; i < m_connectionLimit; i++) {
        if (m_clientConnectionsAddresses.contains(address) || address == 0x0000 || address == 0xFFFF) {
            address++;
        } else {
            break;
        }
    }

    Q_ASSERT_X(!m_clientConnectionsAddresses.contains(address), "TunnelProxyServerConnection", "no free address but the maximum of connections has been reached.");
    return address;
}

QDebug operator<<(QDebug debug, TunnelProxyServerConnection *serverConnection)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TunnelProxyServerConnection(";
    debug.nospace() << serverConnection->serverName() << ", ";
    debug.nospace() << serverConnection->serverUuid().toString() << ", ";
    debug.nospace() << serverConnection->transportClient() << ")";
    return debug;
}

}
