#include "proxyserver.h"
#include "loggingcategories.h"

ProxyServer::ProxyServer(QObject *parent) : QObject(parent)
{

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

void ProxyServer::onClientConnected(const QUuid &clientId)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());
    qCDebug(dcProxyServer()) << "New client connected"  << interface->serverName() << clientId.toString();

    m_unauthenticatedClients.append(clientId);
}

void ProxyServer::onClientDisconnected(const QUuid &clientId)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());
    qCDebug(dcProxyServer()) << "Client disconnected" << interface->serverName() << clientId.toString();

    // Clean up this client since it does not exist any more
    m_unauthenticatedClients.removeAll(clientId);
}

void ProxyServer::onClientDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());
    qCDebug(dcProxyServer()) << "Client data available" << interface->serverName() << clientId.toString() << qUtf8Printable(data);
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
