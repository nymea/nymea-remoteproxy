/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "proxyserver.h"
#include "loggingcategories.h"

#include <QMetaObject>
#include <QVariantList>
#include <QJsonDocument>

namespace remoteproxy {

ProxyServer::ProxyServer(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<ProxyClient *>("ProxyClient *");
    m_jsonRpcServer = new JsonRpcServer(this);
}

ProxyServer::~ProxyServer()
{
    qCDebug(dcProxyServer()) << "Shutting down proxy server";
}

bool ProxyServer::running() const
{
    return m_running;
}

void ProxyServer::registerTransportInterface(TransportInterface *interface)
{
    qCDebug(dcProxyServer()) << "Register transport interface" << interface->serverName();

    if (m_transportInterfaces.contains(interface)) {
        qCWarning(dcProxyServer()) << "Transport interface already registerd.";
        return;
    }

    connect(interface, &TransportInterface::clientConnected, this, &ProxyServer::onClientConnected);
    connect(interface, &TransportInterface::clientDisconnected, this, &ProxyServer::onClientDisconnected);
    connect(interface, &TransportInterface::dataAvailable, this, &ProxyServer::onClientDataAvailable);

    m_transportInterfaces.append(interface);
}

QVariantMap ProxyServer::currentStatistics()
{
    QVariantMap statisticsMap;
    statisticsMap.insert("clientCount", m_proxyClients.count());
    statisticsMap.insert("tunnelCount", m_tunnels.count());

    // Create client list
    QVariantList clientList;
    foreach (ProxyClient *client, m_proxyClients) {
        QVariantMap clientMap;
        clientMap.insert("id", client->clientId().toString());
        clientMap.insert("address", client->peerAddress().toString());
        clientMap.insert("timestamp", client->creationTime());
        clientMap.insert("authenticated", client->isAuthenticated());
        clientMap.insert("tunnelConnected", client->isTunnelConnected());
        clientMap.insert("name", client->name());
        clientMap.insert("uuid", client->uuid());
        clientList.append(clientMap);
    }
    statisticsMap.insert("clients", clientList);

    // Create tunnel list
    QVariantList tunnelList;
    foreach (const TunnelConnection &tunnel, m_tunnels) {
        QVariantMap tunnelMap;
        tunnelMap.insert("clientOne", tunnel.clientOne()->clientId().toString());
        tunnelMap.insert("clientTwo", tunnel.clientTwo()->clientId().toString());
        tunnelMap.insert("timestamp", tunnel.creationTime());
        tunnelList.append(tunnelMap);
    }
    statisticsMap.insert("tunnels", tunnelList);

    return statisticsMap;
}

void ProxyServer::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcProxyServer()) << "The proxy server is now up and running";
    m_running = running;
    emit runningChanged();
}

ProxyClient *ProxyServer::getRemoteClient(ProxyClient *proxyClient)
{
    if (!m_tunnels.contains(proxyClient->token()))
        return nullptr;

    if (proxyClient == m_tunnels.value(proxyClient->token()).clientOne()) {
        return m_tunnels.value(proxyClient->token()).clientTwo();
    } else if (proxyClient == m_tunnels.value(proxyClient->token()).clientTwo()) {
        return m_tunnels.value(proxyClient->token()).clientOne();
    }

    return nullptr;
}

void ProxyServer::sendResponse(TransportInterface *interface, const QUuid &clientId, const QVariantMap &response)
{
    QByteArray data = QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

void ProxyServer::establishTunnel(ProxyClient *firstClient, ProxyClient *secondClient)
{
    qCDebug(dcProxyServer()) << "Create tunnel between authenticated clients:";
    qCDebug(dcProxyServer()) << "    -->" << firstClient << firstClient->name();
    qCDebug(dcProxyServer()) << "    -->" << secondClient << secondClient->name();

    TunnelConnection tunnel(firstClient, secondClient);
    if (!tunnel.isValid()) {
        qCWarning(dcProxyServer()) << "Invalid tunnel. Could not establish connection.";
        //FIXME:
    }

    m_tunnels.insert(tunnel.token(), tunnel);

    // Tell both clients the tunnel has been established
    QVariantMap notificationParamsFirst;
    notificationParamsFirst.insert("name", tunnel.clientTwo()->name());
    notificationParamsFirst.insert("uuid", tunnel.clientTwo()->uuid());

    QVariantMap notificationParamsSecond;
    notificationParamsSecond.insert("name", tunnel.clientOne()->name());
    notificationParamsSecond.insert("uuid", tunnel.clientOne()->uuid());

    // Make sure the proxy is the first one who knows that the tunnel is connected
    firstClient->setTunnelConnected(true);
    secondClient->setTunnelConnected(true);

    // Notify the clients in the next event loop
    QMetaObject::invokeMethod(m_jsonRpcServer, QString("sendNotification").toLatin1().data(), Qt::QueuedConnection,
                              Q_ARG(QString, "ProxyServer"),
                              Q_ARG(QString, "TunnelEstablished"),
                              Q_ARG(QVariantMap, notificationParamsFirst),
                              Q_ARG(ProxyClient *, tunnel.clientOne()));


    QMetaObject::invokeMethod(m_jsonRpcServer, QString("sendNotification").toLatin1().data(), Qt::QueuedConnection,
                              Q_ARG(QString, "ProxyServer"),
                              Q_ARG(QString, "TunnelEstablished"),
                              Q_ARG(QVariantMap, notificationParamsSecond),
                              Q_ARG(ProxyClient *, tunnel.clientTwo()));
}

void ProxyServer::onClientConnected(const QUuid &clientId, const QHostAddress &address)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());

    qCDebug(dcProxyServer()) << "New client connected"  << interface->serverName() << clientId.toString() << address.toString();

    ProxyClient *proxyClient = new ProxyClient(interface, clientId, address, this);
    connect(proxyClient, &ProxyClient::authenticated, this, &ProxyServer::onProxyClientAuthenticated);
    connect(proxyClient, &ProxyClient::tunnelConnected, this, &ProxyServer::onProxyClientTunnelConnected);
    connect(proxyClient, &ProxyClient::timeoutOccured, this, &ProxyServer::onProxyClientTimeoutOccured);

    m_proxyClients.insert(clientId, proxyClient);
    m_jsonRpcServer->registerClient(proxyClient);
}

void ProxyServer::onClientDisconnected(const QUuid &clientId)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());
    qCDebug(dcProxyServer()) << "Client disconnected" << interface->serverName() << clientId.toString();

    if (m_proxyClients.contains(clientId)) {
        ProxyClient *proxyClient = m_proxyClients.take(clientId);

        // Clean up client tables
        if (m_authenticatedClients.values().contains(proxyClient)) {
            m_authenticatedClients.remove(proxyClient->token());
        }

        // Unregister from json rpc server
        m_jsonRpcServer->unregisterClient(proxyClient);

        // Check if
        if (m_tunnels.contains(proxyClient->token())) {
            // There is a tunnel connection for this client, remove the tunnel and disconnect also the other client
            ProxyClient *remoteClient = getRemoteClient(proxyClient);
            m_tunnels.remove(proxyClient->token());
            if (remoteClient) {
                remoteClient->killConnection("Tunnel client disconnected");
            }
        }

        // Delete the proxy client
        proxyClient->deleteLater();
    }

}

void ProxyServer::onClientDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    ProxyClient *proxyClient = m_proxyClients.value(clientId);
    if (!proxyClient) {
        qCWarning(dcProxyServer()) << "Could not find client for uuid" << clientId;
        return;
    }

    // If this client is not authenticated yet, and not tunnel connected, pipe the traffic into the json rpc server
    if (!proxyClient->isAuthenticated() && !proxyClient->isTunnelConnected()) {
        qCDebug(dcProxyServerTraffic()) << "Client data available" << proxyClient << qUtf8Printable(data);
        m_jsonRpcServer->processData(proxyClient, data);
        return;
    }

    // If the client is authenticated, but no tunnel created yet, kill the connection since no addition call is allowed until
    // the tunne is fully established.
    if (proxyClient->isAuthenticated() && !proxyClient->isTunnelConnected()) {
        qCWarning(dcProxyServer()) << "An authenticated client sent data without tunnel connection. This is not allowed.";
        m_jsonRpcServer->unregisterClient(proxyClient);
        // The client is authenticated and tries to send data, this is not allowed.
        proxyClient->killConnection("Data received while authenticated but not remote connected.");
        return;
    }

    if (proxyClient->isAuthenticated() && proxyClient->isTunnelConnected()) {
        ProxyClient *remoteClient = getRemoteClient(proxyClient);
        Q_ASSERT_X(remoteClient, "ProxyServer", "Tunnel existing but not tunnel client available");
        Q_ASSERT_X(m_tunnels.contains(proxyClient->token()), "ProxyServer", "Tunnel connect but not existing");

        qCDebug(dcProxyServerTraffic()) << "Pipe data:";
        qCDebug(dcProxyServerTraffic()) << "    --> from" << proxyClient;
        qCDebug(dcProxyServerTraffic()) << "    --> to" << remoteClient;
        qCDebug(dcProxyServerTraffic()) << "    --> data:" << qUtf8Printable(data);

        remoteClient->sendData(data);
    }
}

void ProxyServer::onProxyClientAuthenticated()
{
    ProxyClient *proxyClient = static_cast<ProxyClient *>(sender());

    qCDebug(dcProxyServer()) << "Client authenticated" << proxyClient;
    qCDebug(dcProxyServer()) << "   name:" << proxyClient->name();
    qCDebug(dcProxyServer()) << "   uuid:" << proxyClient->uuid();

    if (m_tunnels.contains(proxyClient->token())) {
        qCWarning(dcProxyServer()) << "There is already a tunnel connection for this token. A third client is not allowed.";
        // Note: remove the authenticated token, so the current tunnel will not interrupted.
        proxyClient->setToken(QString());
        proxyClient->setAuthenticated(false);
        proxyClient->killConnection("There is already an established tunnel with this token.");
        return;
    }

    // Check if we have an other authenticated client with this token
    if (m_authenticatedClients.keys().contains(proxyClient->token())) {
        // Found a client with this token
        ProxyClient *tunnelEnd = m_authenticatedClients.take(proxyClient->token());
        establishTunnel(tunnelEnd, proxyClient);
    } else {
        // Append and wait for the other client
        m_authenticatedClients.insert(proxyClient->token(), proxyClient);
    }
}

void ProxyServer::onProxyClientTunnelConnected()
{

}

void ProxyServer::onProxyClientTimeoutOccured()
{
    ProxyClient *proxyClient = static_cast<ProxyClient *>(sender());
    qCDebug(dcProxyServer()) << "Timeout occured for" << proxyClient;
    proxyClient->killConnection("Proxy timeout occuret");
}

void ProxyServer::startServer()
{
    qCDebug(dcProxyServer()) << "Start proxy server.";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->startServer();
    }
    setRunning(true);
}

void ProxyServer::stopServer()
{
    qCDebug(dcProxyServer()) << "Stop proxy server.";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->stopServer();
    }
    setRunning(false);
}

}
