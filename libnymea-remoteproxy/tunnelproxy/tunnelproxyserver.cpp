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

#include "tunnelproxyserver.h"
#include "loggingcategories.h"

#include "jsonrpc/tunnelproxyhandler.h"
#include "tunnelproxyserverconnection.h"
#include "tunnelproxyclientconnection.h"

namespace remoteproxy {

TunnelProxyServer::TunnelProxyServer(QObject *parent) :
    QObject(parent)
{
    m_jsonRpcServer = new JsonRpcServer(this);
    m_jsonRpcServer->registerHandler(m_jsonRpcServer);
    m_jsonRpcServer->registerHandler(new TunnelProxyHandler(this));
}

TunnelProxyServer::~TunnelProxyServer()
{

}

bool TunnelProxyServer::running() const
{
    return m_running;
}

void TunnelProxyServer::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcTunnelProxyServer()) << "The proxy tunnel manager is" << (running ? "now up and running." : "not running any more.");
    m_running = running;
    emit runningChanged(m_running);
}

void TunnelProxyServer::registerTransportInterface(TransportInterface *interface)
{
    qCDebug(dcTunnelProxyServer()) << "Register transport interface" << interface->serverName();

    if (m_transportInterfaces.contains(interface)) {
        qCWarning(dcTunnelProxyServer()) << "Transport interface already registerd.";
        return;
    }

    connect(interface, &TransportInterface::clientConnected, this, &TunnelProxyServer::onClientConnected);
    connect(interface, &TransportInterface::clientDisconnected, this, &TunnelProxyServer::onClientDisconnected);
    connect(interface, &TransportInterface::dataAvailable, this, &TunnelProxyServer::onClientDataAvailable);

    m_transportInterfaces.append(interface);
}

TunnelProxyServer::TunnelProxyError TunnelProxyServer::registerServer(const QUuid &clientId, const QUuid &serverUuid, const QString &serverName)
{
    qCDebug(dcTunnelProxyServer()) << "Register new server" << m_proxyClients.value(clientId) << serverName << serverUuid.toString();

    // Check if requested already as client
    TunnelProxyClient *tunnelProxyClient = m_proxyClients.value(clientId);
    if (!tunnelProxyClient) {
        qCWarning(dcTunnelProxyServer()) << "There is no client with client uuid" << clientId.toString();
        return TunnelProxyServer::TunnelProxyErrorInternalServerError;
    }

    // Make sure this client has not been registered as client or re-registration has been called...
    if (tunnelProxyClient->type() != TunnelProxyClient::TypeNone) {
        qCWarning(dcTunnelProxyServer()) << "Client tried to register as server but has already been registerd as" << tunnelProxyClient->type();
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    tunnelProxyClient->setType(TunnelProxyClient::TypeServer);
    tunnelProxyClient->setUuid(serverUuid);
    tunnelProxyClient->setName(serverName);

    TunnelProxyServerConnection *serverConnection = new TunnelProxyServerConnection(tunnelProxyClient, serverUuid, serverName, this);
    m_tunnelProxyServerConnections.insert(serverUuid, serverConnection);

    return TunnelProxyServer::TunnelProxyErrorNoError;
}

TunnelProxyServer::TunnelProxyError TunnelProxyServer::registerClient(const QUuid &clientId, const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid)
{
    qCDebug(dcTunnelProxyServer()) << "Register new client" << m_proxyClients.value(clientId) << clientName << clientUuid.toString() << "--> server" << serverUuid.toString();

    TunnelProxyClient *tunnelProxyClient = m_proxyClients.value(clientId);
    if (!tunnelProxyClient) {
        qCWarning(dcTunnelProxyServer()) << "There is no client with client uuid" << clientId.toString();
        return TunnelProxyServer::TunnelProxyErrorInternalServerError;
    }

    // Make sure this client has not been registered as client or re-registration has been called...
    if (tunnelProxyClient->type() != TunnelProxyClient::TypeNone) {
        qCWarning(dcTunnelProxyServer()) << "Client tried to register as client but has already been registerd as" << tunnelProxyClient->type();
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    // Get the desired server connection
    TunnelProxyServerConnection *serverConnection = m_tunnelProxyServerConnections.value(serverUuid);
    if (!serverConnection) {
        qCWarning(dcTunnelProxyServer()) << "There is no server registered with server uuid" << serverUuid.toString();
        return TunnelProxyServer::TunnelProxyErrorServerNotFound;
    }

    if (m_tunnelProxyClientConnections.contains(clientUuid)) {
        qCWarning(dcTunnelProxyServer()) << "There is a client already registered with client uuid" << clientUuid.toString();
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    // Not registered yet, we have a connected server for the requested server uuid
    tunnelProxyClient->setType(TunnelProxyClient::TypeClient);
    tunnelProxyClient->setUuid(clientUuid);
    tunnelProxyClient->setName(clientName);

    TunnelProxyClientConnection *clientConnection = new TunnelProxyClientConnection(tunnelProxyClient, clientUuid, clientName, serverUuid);
    m_tunnelProxyClientConnections.insert(clientUuid, clientConnection);

    // TODO: register on the server and wait for te aprovement from the server

    return TunnelProxyServer::TunnelProxyErrorNoError;
}

void TunnelProxyServer::startServer()
{
    qCDebug(dcTunnelProxyServer()) << "Starting tunnel proxy...";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->startServer();
    }
    setRunning(true);
}

void TunnelProxyServer::stopServer()
{
    qCDebug(dcTunnelProxyServer()) << "Stopping tunnel proxy...";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->stopServer();
    }
    setRunning(false);
}

void TunnelProxyServer::tick()
{

}

void TunnelProxyServer::onClientConnected(const QUuid &clientId, const QHostAddress &address)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());

    qCDebug(dcTunnelProxyServer()) << "New client connected"  << interface->serverName() << clientId.toString() << address.toString();
    TunnelProxyClient *tunnelProxyClient = new TunnelProxyClient(interface, clientId, address, this);
    m_proxyClients.insert(clientId, tunnelProxyClient);
    m_jsonRpcServer->registerClient(tunnelProxyClient);
}

void TunnelProxyServer::onClientDisconnected(const QUuid &clientId)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());
    qCDebug(dcTunnelProxyServer()) << "Client disconnected" << interface->serverName() << clientId.toString();

    TunnelProxyClient *tunnelProxyClient = m_proxyClients.take(clientId);
    if (!tunnelProxyClient) {
        qCWarning(dcTunnelProxyServer()) << "Unknown client disconnected from proxy server." << clientId.toString();
        return;
    }

    if (tunnelProxyClient->type() == TunnelProxyClient::TypeServer) {
        TunnelProxyServerConnection *serverConnection = m_tunnelProxyServerConnections.take(tunnelProxyClient->uuid());
        if (!serverConnection) {
            qCWarning(dcTunnelProxyServer()) << "Could not find server connection for disconnected tunnel proxy client claiming to be a server.";
        } else {
            // TODO: kill all related clients

            serverConnection->deleteLater();
        }
    }

    if (tunnelProxyClient->type() == TunnelProxyClient::TypeClient) {
        TunnelProxyClientConnection *clientConnection = m_tunnelProxyClientConnections.take(tunnelProxyClient->uuid());

        if (!clientConnection) {
            qCWarning(dcTunnelProxyServer()) << "Could not find client connection for disconnected tunnel proxy client claiming to be a client.";
        } else {
            // TODO: remove from server

            clientConnection->deleteLater();
        }
    }

    // Unregister from json rpc server
    m_jsonRpcServer->unregisterClient(tunnelProxyClient);

    // Delete the proxy client
    tunnelProxyClient->deleteLater();
}

void TunnelProxyServer::onClientDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    TunnelProxyClient *tunnelProxyClient = m_proxyClients.value(clientId);
    if (!tunnelProxyClient) {
        qCWarning(dcTunnelProxyServer()) << "Data received but could not find client for uuid" << clientId;
        return;
    }

    qCDebug(dcTunnelProxyServerTraffic()) << "Client data available" << tunnelProxyClient << qUtf8Printable(data);

    // TODO: verify if encoded and for whom this data is... 0x0000 is for the json rpc handler...

    m_jsonRpcServer->processData(tunnelProxyClient, data);
}

}
