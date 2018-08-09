#include "websocketserver.h"
#include "loggingcategories.h"

#include <QCoreApplication>

namespace remoteproxy {

WebSocketServer::WebSocketServer(const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_sslConfiguration(sslConfiguration)
{
    m_serverName = "Websocket server";
}

WebSocketServer::~WebSocketServer()
{
    stopServer();
}

QUrl WebSocketServer::serverUrl() const
{
    return m_serverUrl;
}

void WebSocketServer::setServerUrl(const QUrl &serverUrl)
{
    m_serverUrl = serverUrl;
}

bool WebSocketServer::running() const
{
    return m_server->isListening();
}

QSslConfiguration WebSocketServer::sslConfiguration() const
{
    return m_sslConfiguration;
}

void WebSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QWebSocket *client = nullptr;
    client = m_clientList.value(clientId);
    if (client) {
        qCDebug(dcWebSocketServerTraffic()) << "--> Sending data to client:" << data;
        client->sendTextMessage(data + '\n');
    } else {
        qCWarning(dcWebSocketServer()) << "Client" << clientId << "unknown to this transport";
    }
}

void WebSocketServer::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    foreach (const QUuid &client, clients) {
        sendData(client, data);
    }
}

void WebSocketServer::killClientConnection(const QUuid &clientId)
{
    QWebSocket *client = m_clientList.value(clientId);
    if (!client)
        return;

    qCWarning(dcWebSocketServer()) << "Kill client connection" << clientId.toString();
    client->close(QWebSocketProtocol::CloseCodeBadOperation);
}

void WebSocketServer::onClientConnected()
{
    // Got a new client connected
    QWebSocket *client = m_server->nextPendingConnection();

    // Check websocket version
    if (client->version() != QWebSocketProtocol::Version13) {
        qCWarning(dcWebSocketServer()) << "Client with invalid protocol version" << client->version() << ". Rejecting.";
        client->close(QWebSocketProtocol::CloseCodeProtocolError, QString("invalid protocol version: %1 != Supported Version 13").arg(client->version()));
        delete client;
        return;
    }

    // Create new uuid for this connection
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcWebSocketServer()) << "New client connected:" << client << client->peerAddress().toString() << clientId.toString();

    // Append the new client to the client list
    m_clientList.insert(clientId, client);

    connect(client, SIGNAL(pong(quint64,QByteArray)), this, SLOT(onPing(quint64,QByteArray)));
    connect(client, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));
    connect(client, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onClientError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

    emit clientConnected(clientId);
}

void WebSocketServer::onClientDisconnected()
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    QUuid clientId = m_clientList.key(client);

    qCDebug(dcWebSocketServer()) << "Client disconnected:" << client << client->peerAddress().toString() << clientId.toString();

    m_clientList.take(clientId)->deleteLater();
    emit clientDisconnected(clientId);
}

void WebSocketServer::onBinaryMessageReceived(const QByteArray &data)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServerTraffic()) << "<-- Binary message from" << client->peerAddress().toString() << ":" << data;
}

void WebSocketServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServerTraffic()) << "Text message from" << client->peerAddress().toString() << ":" << message;
    emit dataAvailable(m_clientList.key(client), message.toUtf8());
}

void WebSocketServer::onClientError(QAbstractSocket::SocketError error)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCWarning(dcWebSocketServer()) << "Client error occured:" << error << client->errorString();
}

void WebSocketServer::onServerError(QAbstractSocket::SocketError error)
{
    qCWarning(dcWebSocketServer()) << "Server error occured:" << error << m_server->errorString();
}

void WebSocketServer::onPing(quint64 elapsedTime, const QByteArray &payload)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServer) << "ping response" << client->peerAddress() << elapsedTime << payload;
}

bool WebSocketServer::startServer()
{
    m_server = new QWebSocketServer(QCoreApplication::applicationName(), QWebSocketServer::SecureMode, this);
    m_server->setSslConfiguration(sslConfiguration());

    connect (m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onClientConnected);
    connect (m_server, &QWebSocketServer::acceptError, this, &WebSocketServer::onServerError);

    qCDebug(dcWebSocketServer()) << "Starting server" << m_server->serverName() << serverUrl().toString();
    if (!m_server->listen(QHostAddress(m_serverUrl.host()), static_cast<quint16>(serverUrl().port()))) {
        qCWarning(dcWebSocketServer()) << "Server" << m_server->serverName() << "could not listen on" << serverUrl().toString();
        return false;
    }

    qCDebug(dcWebSocketServer()) << "Server started successfully.";
    return true;
}

bool WebSocketServer::stopServer()
{
    // Clean up client connections
    foreach (QWebSocket *client, m_clientList.values()) {
        client->close(QWebSocketProtocol::CloseCodeNormal, "Stop server");
    }

    // Delete the server object
    if (m_server) {
        qCDebug(dcWebSocketServer()) << "Stop server" << m_server->serverName() << serverUrl().toString();
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }

    return true;
}

}
