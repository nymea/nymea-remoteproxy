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

#include "tunnelproxysocket.h"
#include "proxyconnection.h"
#include "tunnelproxysocketserver.h"
#include "../common/slipdataprocessor.h"

namespace remoteproxyclient {

TunnelProxySocket::TunnelProxySocket(ProxyConnection *connection, TunnelProxySocketServer *socketServer, const QString &clientName, const QUuid &clientUuid, const QHostAddress &clientPeerAddress, quint16 socketAddress, QObject *parent) :
    QObject(parent),
    m_connection(connection),
    m_socketServer(socketServer),
    m_clientName(clientName),
    m_clientUuid(clientUuid),
    m_clientPeerAddress(clientPeerAddress),
    m_socketAddress(socketAddress)
{

}

QUuid TunnelProxySocket::clientUuid() const
{
    return m_clientUuid;
}

QString TunnelProxySocket::clientName() const
{
    return m_clientName;
}

QHostAddress TunnelProxySocket::clientPeerAddress() const
{
    return m_clientPeerAddress;
}

quint16 TunnelProxySocket::socketAddress() const
{
    return m_socketAddress;
}

bool TunnelProxySocket::connected() const
{
    return m_connected;
}

void TunnelProxySocket::writeData(const QByteArray &data)
{
    SlipDataProcessor::Frame frame;
    frame.socketAddress = m_socketAddress;
    frame.data = data;
    m_connection->sendData(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame)));
}

void TunnelProxySocket::disconnectSocket()
{
    m_socketServer->requestSocketDisconnect(m_socketAddress);
}

void TunnelProxySocket::setDisconnected()
{
    m_connected = false;
    emit connectedChanged(false);
    emit disconnected();
}

QDebug operator<<(QDebug debug, TunnelProxySocket *tunnelProxySocket)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TunnelProxySocket(";
    debug.nospace() << tunnelProxySocket->clientName() << ", ";
    debug.nospace() << tunnelProxySocket->clientUuid().toString() << ", ";
    debug.nospace() << tunnelProxySocket->clientPeerAddress().toString() << ", ";
    debug.nospace() << tunnelProxySocket->socketAddress() << ")";
    return debug;
}

}
