/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
