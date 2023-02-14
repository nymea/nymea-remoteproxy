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

#include "tunnelproxyserver.h"
#include "loggingcategories.h"

#include "jsonrpc/tunnelproxyhandler.h"
#include "tunnelproxyserverconnection.h"
#include "tunnelproxyclientconnection.h"

#include "../common/slipdataprocessor.h"

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
        tunnelProxyClient->killConnectionAfterResponse("Already registered");
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    // This client has been registered successfully.
    // Make sure it does not get disconnected any more because due to inactivity.
    tunnelProxyClient->makeClientActive();

    tunnelProxyClient->setType(TunnelProxyClient::TypeServer);
    tunnelProxyClient->setUuid(serverUuid);
    tunnelProxyClient->setName(serverName);

    // Enable SLIP from now on
    tunnelProxyClient->enableSlipAfterResponse();

    TunnelProxyServerConnection *serverConnection = new TunnelProxyServerConnection(tunnelProxyClient, serverUuid, serverName, this);
    m_tunnelProxyServerConnections.insert(serverUuid, serverConnection);

    return TunnelProxyServer::TunnelProxyErrorNoError;
}

TunnelProxyServer::TunnelProxyError TunnelProxyServer::registerClient(const QUuid &clientId, const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid)
{
    TunnelProxyClient *tunnelProxyClient = m_proxyClients.value(clientId);
    if (!tunnelProxyClient) {
        qCWarning(dcTunnelProxyServer()) << "There is no client with client uuid" << clientId.toString();
        return TunnelProxyServer::TunnelProxyErrorInternalServerError;
    }

    // Make sure this client has not been registered as client or re-registration has been called...
    if (tunnelProxyClient->type() != TunnelProxyClient::TypeNone) {
        qCWarning(dcTunnelProxyServer()) << "Client tried to register as client but has already been registerd as" << tunnelProxyClient->type();
        tunnelProxyClient->killConnectionAfterResponse("Already registered");
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    if (m_tunnelProxyClientConnections.contains(clientUuid)) {
        qCWarning(dcTunnelProxyServer()) << "There is a client already registered with client uuid" << clientUuid.toString();
        tunnelProxyClient->killConnectionAfterResponse("Already registered");
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    // Get the desired server connection
    TunnelProxyServerConnection *serverConnection = m_tunnelProxyServerConnections.value(serverUuid);
    if (!serverConnection) {
        qCWarning(dcTunnelProxyServer()) << "There is no server registered with server uuid" << serverUuid.toString();
        tunnelProxyClient->killConnectionAfterResponse("Unknown server");
        return TunnelProxyServer::TunnelProxyErrorServerNotFound;
    }

    // This client has been registered successfully.
    // Make sure it does not get disconnected any more because due to inactivity.
    tunnelProxyClient->makeClientActive();

    // Not registered yet, we have a connected server for the requested server uuid
    tunnelProxyClient->setType(TunnelProxyClient::TypeClient);
    tunnelProxyClient->setUuid(clientUuid);
    tunnelProxyClient->setName(clientName);

    TunnelProxyClientConnection *clientConnection = new TunnelProxyClientConnection(tunnelProxyClient, clientUuid, clientName, this);
    clientConnection->setServerConnection(serverConnection);
    m_tunnelProxyClientConnections.insert(clientUuid, clientConnection);

    qCDebug(dcTunnelProxyServer()) << "Register client" << clientConnection << "-->" << serverConnection;
    serverConnection->registerClientConnection(clientConnection);

    // Tell the server a new client want's to connect
    QVariantMap params;
    params.insert("clientName", tunnelProxyClient->name());
    params.insert("clientUuid", tunnelProxyClient->uuid().toString());
    params.insert("clientPeerAddress", tunnelProxyClient->peerAddress().toString());
    params.insert("socketAddress", clientConnection->socketAddress());
    m_jsonRpcServer->sendNotification("TunnelProxy", "ClientConnected", params, serverConnection->transportClient());

    // Note: check if a confirmation from the server would be needed, for rejection or limit or something. For now they are directly connected.

    return TunnelProxyServer::TunnelProxyErrorNoError;
}

TunnelProxyServer::TunnelProxyError TunnelProxyServer::disconnectClient(const QUuid &clientId, quint16 socketAddress)
{
    TunnelProxyClient *tunnelProxyClient = m_proxyClients.value(clientId);
    if (!tunnelProxyClient) {
        qCWarning(dcTunnelProxyServer()) << "There is no client with client uuid" << clientId.toString();
        return TunnelProxyServer::TunnelProxyErrorInternalServerError;
    }

    if (tunnelProxyClient->type() != TunnelProxyClient::TypeServer) {
        qCWarning(dcTunnelProxyServer()) << "Client tried to disconnect a client but has not been registerd as server" << tunnelProxyClient->type();
        tunnelProxyClient->killConnectionAfterResponse("Already registered");
        return TunnelProxyServer::TunnelProxyErrorAlreadyRegistered;
    }

    TunnelProxyServerConnection *serverConnection = m_tunnelProxyServerConnections.value(tunnelProxyClient->uuid());
    if (!serverConnection) {
        qCWarning(dcTunnelProxyServer()) << "Could not find server connection for" << tunnelProxyClient;
        tunnelProxyClient->killConnectionAfterResponse("Internal server error");
        return TunnelProxyServer::TunnelProxyErrorInternalServerError;
    }

    TunnelProxyClientConnection *clientConnection = serverConnection->getClientConnection(socketAddress);
    if (!clientConnection) {
        qCWarning(dcTunnelProxyServer()) << "Could not find client connection for socket address" << socketAddress << "on" << serverConnection;
        return TunnelProxyServer::TunnelProxyErrorUnknownSocketAddress;
    }

    clientConnection->transportClient()->killConnection("Server requested disconnect.");
    return TunnelProxyServer::TunnelProxyErrorNoError;
}

QVariantMap TunnelProxyServer::currentStatistics()
{
    QVariantMap statisticsMap;
    statisticsMap.insert("totalClientCount", m_proxyClients.count());
    statisticsMap.insert("serverConnectionsCount", m_tunnelProxyServerConnections.count());
    statisticsMap.insert("clientConnectionsCount", m_tunnelProxyClientConnections.count());
    QVariantMap transports;
    foreach (TransportInterface *transportInterface, m_transportInterfaces) {
        transports.insert(transportInterface->name(), transportInterface->connectionsCount());
    }
    statisticsMap.insert("transports", transports);
    statisticsMap.insert("troughput", m_troughput);

    QVariantList tunnelConnections;
    foreach (TunnelProxyServerConnection *serverConnection, m_tunnelProxyServerConnections) {

        // Show only active clients
        if (serverConnection->clientConnections().isEmpty())
            continue;

        QVariantMap serverMap;
        serverMap.insert("id", serverConnection->transportClient()->clientId().toString());
        serverMap.insert("address", serverConnection->transportClient()->peerAddress().toString());
        serverMap.insert("timestamp", serverConnection->transportClient()->creationTime());
        serverMap.insert("name", serverConnection->transportClient()->name());
        serverMap.insert("serverUuid", serverConnection->transportClient()->uuid());
        serverMap.insert("rxDataCount", serverConnection->transportClient()->rxDataCount());
        serverMap.insert("txDataCount", serverConnection->transportClient()->txDataCount());

        QVariantList clientList;
        foreach (TunnelProxyClientConnection *clientConnection, serverConnection->clientConnections()) {
            QVariantMap clientMap;
            clientMap.insert("id", clientConnection->transportClient()->clientId().toString());
            clientMap.insert("address", clientConnection->transportClient()->peerAddress().toString());
            clientMap.insert("timestamp", clientConnection->transportClient()->creationTime());
            clientMap.insert("name", clientConnection->transportClient()->name());
            clientMap.insert("clientUuid", clientConnection->transportClient()->uuid());
            clientMap.insert("rxDataCount", clientConnection->transportClient()->rxDataCount());
            clientMap.insert("txDataCount", clientConnection->transportClient()->txDataCount());
            clientList.append(clientMap);
        }
        serverMap.insert("clientConnections", clientList);
        tunnelConnections.append(serverMap);
    }

    statisticsMap.insert("tunnelConnections", tunnelConnections);

    return statisticsMap;
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
    m_troughput = m_troughputCounter;
    m_troughputCounter = 0;
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
            foreach (TunnelProxyClientConnection *clientConnection, serverConnection->clientConnections()) {
                serverConnection->unregisterClientConnection(clientConnection);
                clientConnection->setSocketAddress(0xFFFF);
                clientConnection->transportClient()->killConnection("Server disconnected");
            }

            serverConnection->deleteLater();
        }
    }

    if (tunnelProxyClient->type() == TunnelProxyClient::TypeClient) {
        TunnelProxyClientConnection *clientConnection = m_tunnelProxyClientConnections.take(tunnelProxyClient->uuid());
        if (!clientConnection) {
            qCWarning(dcTunnelProxyServer()) << "Could not find client connection for disconnected tunnel proxy client claiming to be a client.";
        } else {
            TunnelProxyServerConnection *serverConnection = clientConnection->serverConnection();
            if (serverConnection) {
                QVariantMap params;
                params.insert("socketAddress", clientConnection->socketAddress());
                serverConnection->unregisterClientConnection(clientConnection);
                m_jsonRpcServer->sendNotification("TunnelProxy", "ClientDisconnected", params, serverConnection->transportClient());
            }

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
    tunnelProxyClient->addRxDataCount(data.count());

    if (tunnelProxyClient->type() == TunnelProxyClient::TypeClient) {
        // Send the data to the server using slip encoded frame
        TunnelProxyClientConnection *clientConnection = m_tunnelProxyClientConnections.value(tunnelProxyClient->uuid());
        if (!clientConnection) {
            qCWarning(dcTunnelProxyServer()) << "Could not find a client connection for client uuid" << tunnelProxyClient->uuid().toString();
            // FIXME: check what do with this client
            return;
        }

        Q_ASSERT_X(clientConnection->serverConnection(), "TunnelProxyServer", "The client has not been registered to a server connection");

        SlipDataProcessor::Frame frame;
        frame.socketAddress = clientConnection->socketAddress();
        frame.data = data;
        qCDebug(dcTunnelProxyServerTraffic()) << "--> Tunnel data to server socket address" << clientConnection->socketAddress() << "to" << clientConnection->serverConnection() << "\n" << data;
        QByteArray rawData = SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame));
        clientConnection->serverConnection()->transportClient()->sendData(rawData);
        clientConnection->serverConnection()->transportClient()->addTxDataCount(rawData.count());
        m_troughputCounter += data.count();

    } else if (tunnelProxyClient->type() == TunnelProxyClient::TypeServer) {
        // Data coming from a connected server connection
        if (tunnelProxyClient->slipEnabled()) {
            // Unpack SLIP data, get address, pipe to client or give it to the json rpc server if address 0x0000
            // Handle packet fragmentation
            QList<QByteArray> frames = tunnelProxyClient->processData(data);
            foreach (const QByteArray &frameData, frames) {
                SlipDataProcessor::Frame frame = SlipDataProcessor::parseFrame(frameData);

                if (frame.socketAddress == 0x0000) {
                    qCDebug(dcTunnelProxyServerTraffic()) << "Received frame for the JSON server" << tunnelProxyClient;
                    m_jsonRpcServer->processDataPacket(tunnelProxyClient, frame.data);
                } else {
                    // This data seems to be for a client with the given address
                    TunnelProxyServerConnection *serverConnection = m_tunnelProxyServerConnections.value(tunnelProxyClient->uuid());
                    if (!serverConnection) {
                        qCWarning(dcTunnelProxyServer()) << "Could not find server connection for" << tunnelProxyClient;
                        continue;
                    }

                    // Get the client and send the data to the client
                    TunnelProxyClientConnection *clientConnection = serverConnection->getClientConnection(frame.socketAddress);
                    if (!clientConnection) {
                        qCWarning(dcTunnelProxyServer()) << "The server connection wants to send data to a client connection which has not been registered to the server.";
                        // FIXME: tell the server this client does not exist
                        return;
                    }

                    qCDebug(dcTunnelProxyServerTraffic()) << "--> Tunnel data from server socket" << frame.socketAddress << "to" << clientConnection <<  "\n" << frame.data;
                    clientConnection->transportClient()->sendData(frame.data);
                    clientConnection->transportClient()->addTxDataCount(frame.data.count());
                    m_troughputCounter += frame.data.count();
                }
            }
        } else {
            m_jsonRpcServer->processData(tunnelProxyClient, data);
        }

    } else {
        // Not registered yet or doing other stuff...let the JSON RPC server handle this data
        m_jsonRpcServer->processData(tunnelProxyClient, data);
    }
}

}
