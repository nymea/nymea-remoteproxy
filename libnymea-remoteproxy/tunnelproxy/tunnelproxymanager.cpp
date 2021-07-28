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

#include "tunnelproxymanager.h"
#include "loggingcategories.h"

#include "jsonrpc/tunnelproxyhandler.h"

namespace remoteproxy {

TunnelProxyManager::TunnelProxyManager(QObject *parent) :
    QObject(parent)
{
    m_jsonRpcServer = new JsonRpcServer(this);
    m_jsonRpcServer->registerHandler(m_jsonRpcServer);
    m_jsonRpcServer->registerHandler(new TunnelProxyHandler(this));

}

TunnelProxyManager::~TunnelProxyManager()
{

}

bool TunnelProxyManager::running() const
{
    return m_running;
}

void TunnelProxyManager::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcTunnelProxyManager()) << "The proxy tunnel manager is" << (running ? "now up and running." : "not running any more.");
    m_running = running;
    emit runningChanged(m_running);
}

void TunnelProxyManager::registerTransportInterface(TransportInterface *interface)
{
    qCDebug(dcTunnelProxyManager()) << "Register transport interface" << interface->serverName();

    if (m_transportInterfaces.contains(interface)) {
        qCWarning(dcTunnelProxyManager()) << "Transport interface already registerd.";
        return;
    }

    connect(interface, &TransportInterface::clientConnected, this, &TunnelProxyManager::onClientConnected);
    connect(interface, &TransportInterface::clientDisconnected, this, &TunnelProxyManager::onClientDisconnected);
    connect(interface, &TransportInterface::dataAvailable, this, &TunnelProxyManager::onClientDataAvailable);

    m_transportInterfaces.append(interface);
}

TunnelProxyManager::Error TunnelProxyManager::registerServer(const QUuid &clientId, const QUuid &serverUuid, const QString &serverName)
{
    qCDebug(dcTunnelProxyManager()) << "Register new server" << m_proxyClients.value(clientId) << serverName << serverUuid.toString();

    // TODO: check if uuid already exists

    // Check if requested already as client


    TunnelProxyServer *proxyServer = new TunnelProxyServer(m_proxyClients.value(clientId), serverUuid, serverName);
    m_proxyClientsTunnelServer.insert(clientId, proxyServer);
    m_tunnelServers.insert(proxyServer->serverUuid(), proxyServer);

    return ErrorNoError;
}

void TunnelProxyManager::startServer()
{
    qCDebug(dcTunnelProxyManager()) << "Starting tunnel proxy manager...";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->startServer();
    }
    setRunning(true);
}

void TunnelProxyManager::stopServer()
{
    qCDebug(dcTunnelProxyManager()) << "Stopping tunnel proxy server...";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->stopServer();
    }
    setRunning(false);
}

void TunnelProxyManager::tick()
{

}

void TunnelProxyManager::onClientConnected(const QUuid &clientId, const QHostAddress &address)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());

    qCDebug(dcTunnelProxyManager()) << "New client connected"  << interface->serverName() << clientId.toString() << address.toString();

    ProxyClient *proxyClient = new ProxyClient(interface, clientId, address, this);
    m_proxyClients.insert(clientId, proxyClient);
    m_jsonRpcServer->registerClient(proxyClient);
}

void TunnelProxyManager::onClientDisconnected(const QUuid &clientId)
{
    Q_UNUSED(clientId)
}

void TunnelProxyManager::onClientDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    Q_UNUSED(clientId)
    Q_UNUSED(data)
}

}
