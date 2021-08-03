/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2021, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
    m_clientConnections.insert(clientConnection->clientUuid(), clientConnection);
    quint16 socketAddress = getFreeAddress();
    clientConnection->setSocketAddress(socketAddress);
    m_clientConnectionsAddresses.insert(socketAddress, clientConnection);
}

void TunnelProxyServerConnection::unregisterClientConnection(TunnelProxyClientConnection *clientConnection)
{
    m_clientConnections.remove(clientConnection->clientUuid());
    m_clientConnectionsAddresses.remove(clientConnection->socketAddress());
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
    debug.nospace() << "TunnelProxyServerConnection(";
    debug.nospace() << serverConnection->serverName() << ", ";
    debug.nospace() << serverConnection->serverUuid().toString() << ", ";
    debug.nospace() << serverConnection->transportClient() << ")";
    return debug.space();
}

}
