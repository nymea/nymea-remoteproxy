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
    m_proxyClients.insert(clientId, proxyClient);

    m_jsonRpcServer->registerClient(proxyClient);
}

void ProxyServer::onClientDisconnected(const QUuid &clientId)
{
    TransportInterface *interface = static_cast<TransportInterface *>(sender());
    qCDebug(dcProxyServer()) << "Client disconnected" << interface->serverName() << clientId.toString();

    if (m_proxyClients.contains(clientId)) {
        ProxyClient *proxyClient = m_proxyClients.take(clientId);
        m_jsonRpcServer->unregisterClient(proxyClient);
        proxyClient->deleteLater();
        // Check if client is in tunnel and clean up tunnel
        // Disconnect also the other tunnel client

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
    if (proxyClient->tunnelConnected()) {
        // Pipe the data
    } else {
        // Pipe data into json rpc server
        m_jsonRpcServer->processData(proxyClient, data);
    }
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
