/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
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
