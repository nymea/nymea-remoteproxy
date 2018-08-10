#include "proxyserver.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace remoteproxy {

ProxyServer::ProxyServer(QObject *parent) : QObject(parent)
{
    m_jsonRpcServer = new JsonRpcServer(this);

}

ProxyServer::~ProxyServer()
{
    qCDebug(dcProxyServer()) << "Shutting down proxy server";
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

void ProxyServer::sendResponse(TransportInterface *interface, const QUuid &clientId, const QVariantMap &response)
{
    QByteArray data = QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

void ProxyServer::onClientConnected(const QUuid &clientId)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());

    qCDebug(dcProxyServer()) << "New client connected"  << interface->serverName() << clientId.toString();

    ProxyClient *proxyClient = new ProxyClient(interface, clientId, this);
    connect(proxyClient, &ProxyClient::authenticated, this, &ProxyServer::onProxyClientAuthenticated);
    connect(proxyClient, &ProxyClient::tunnelConnected, this, &ProxyServer::onProxyClientTunnelConnected);

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

        // Delete the proxy client
        proxyClient->deleteLater();

        // TODO: Disconnect also the other tunnel client
    }

    // TODO: Clean up this client since it does not exist any more
}

void ProxyServer::onClientDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    ProxyClient *proxyClient = m_proxyClients.value(clientId);
    if (!proxyClient) {
        qCWarning(dcProxyServer()) << "Could not find client for uuid" << clientId;
        return;
    }

    qCDebug(dcProxyServer()) << "Client data available" << proxyClient << qUtf8Printable(data);

    // If this client is not authenticated yet, and not tunnel connected, pipe the traffic into the json rpc server
    if (!proxyClient->isAuthenticated() && !proxyClient->isTunnelConnected()) {
        m_jsonRpcServer->processData(proxyClient, data);
        return;
    }

    // If the client is authenticated, but no tunnel created yet, kill the connection since no addition call is allowed until
    // the tunne is fully established.
    if (proxyClient->isAuthenticated() && !proxyClient->isTunnelConnected()) {
        qCWarning(dcProxyServer()) << "An authenticated client sent data without tunnel connection. This is not allowed.";
        m_jsonRpcServer->unregisterClient(proxyClient);
        // The client is authenticated and tries to send data, this is not allowed.
        proxyClient->interface()->killClientConnection(proxyClient->clientId(), "Data received while authenticated but not remote connected.");
        return;
    }

    if (proxyClient->isAuthenticated() && proxyClient->isTunnelConnected()) {
        // TODO: Pipe the traffic to the tunnel client
    }
}

void ProxyServer::onProxyClientAuthenticated()
{
    ProxyClient *proxyClient = static_cast<ProxyClient *>(sender());
    qCDebug(dcProxyServer()) << "Client authenticated" << proxyClient;
    m_authenticatedClients.insert(proxyClient->token(), proxyClient);
}

void ProxyServer::onProxyClientTunnelConnected()
{

}

void ProxyServer::startServer()
{
    qCDebug(dcProxyServer()) << "Start proxy server.";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->startServer();
    }
}

void ProxyServer::stopServer()
{
    qCDebug(dcProxyServer()) << "Stop proxy server.";
    foreach (TransportInterface *interface, m_transportInterfaces) {
        interface->stopServer();
    }
}

}
