#include "proxyserver.h"
#include "loggingcategories.h"

#include <QMetaObject>
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
        // FIXME
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
        proxyClient->interface()->killClientConnection(proxyClient->clientId(), "Data received while authenticated but not remote connected.");
        return;
    }

    if (proxyClient->isAuthenticated() && proxyClient->isTunnelConnected()) {
        // Check if there is realy a tunnel for this client
        if (!m_tunnels.contains(proxyClient->token())) {
            // FIXME: kill all clients, something went wrong
        }

        ProxyClient *remoteClient = getRemoteClient(proxyClient);
        if (!remoteClient) {
            // FIXME: kill all clients, something went wrong
        }

        qCDebug(dcProxyServerTraffic()) << "Pipe data:";
        qCDebug(dcProxyServerTraffic()) << "    --> from" << proxyClient;
        qCDebug(dcProxyServerTraffic()) << "    --> to" << remoteClient;
        qCDebug(dcProxyServerTraffic()) << "    --> data:" << qUtf8Printable(data);

        remoteClient->interface()->sendData(remoteClient->clientId(), data);
    }
}

void ProxyServer::onProxyClientAuthenticated()
{
    ProxyClient *proxyClient = static_cast<ProxyClient *>(sender());

    qCDebug(dcProxyServer()) << "Client authenticated" << proxyClient;
    qCDebug(dcProxyServer()) << "   name:" << proxyClient->name();
    qCDebug(dcProxyServer()) << "   uuid:" << proxyClient->uuid();

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
