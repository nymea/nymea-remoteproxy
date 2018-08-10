#include "websocketconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientWebSocket, "RemoteProxyClientWebSocket")

namespace remoteproxyclient {

WebSocketConnection::WebSocketConnection(QObject *parent) :
    ProxyConnection(parent)
{
    m_webSocket = new QWebSocket("libnymea-remoteproxyclient", QWebSocketProtocol::Version13, this);

    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketConnection::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketConnection::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketConnection::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketConnection::onBinaryMessageReceived);

    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError(QList<QSslError>)));
    connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
}

WebSocketConnection::~WebSocketConnection()
{

}

QUrl WebSocketConnection::serverUrl() const
{
    return m_serverUrl;
}

void WebSocketConnection::sendData(const QByteArray &data)
{
    m_webSocket->sendTextMessage(QString::fromUtf8(data));
}

bool WebSocketConnection::isConnected()
{
    return m_webSocket->state() == QAbstractSocket::ConnectedState;
}

void WebSocketConnection::onConnected()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Connected with" << m_webSocket->requestUrl().toString();
    emit connectedChanged(true);
}

void WebSocketConnection::onDisconnected()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnected from" << m_webSocket->requestUrl().toString();
    emit connectedChanged(false);
}

void WebSocketConnection::onError(QAbstractSocket::SocketError error)
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket error occured" << error << m_webSocket->errorString();
}

void WebSocketConnection::onSslError(const QList<QSslError> &errors)
{
    if (allowSslErrors()) {
        qCDebug(dcRemoteProxyClientWebSocket()) << "Ignore ssl errors because the developer explicitly allowed to use an insecure connection.";
        m_webSocket->ignoreSslErrors();
    } else {
        qCWarning(dcRemoteProxyClientWebSocket()) << "SSL errors occured:";
        foreach (const QSslError sslError, errors) {
            qCWarning(dcRemoteProxyClientWebSocket()) << "    -->" << static_cast<int>(sslError.error()) << sslError.errorString();
        }
        emit sslErrorOccured();
    }
}

void WebSocketConnection::onStateChanged(QAbstractSocket::SocketState state)
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket state changed" << state;
}

void WebSocketConnection::onTextMessageReceived(const QString &message)
{
    emit dataReceived(message.toUtf8());
}

void WebSocketConnection::onBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message)
}

void WebSocketConnection::connectServer(const QHostAddress &address, quint16 port)
{
    if (isConnected()) {
        m_webSocket->close();
    }

    QUrl url;
    url.setScheme("wss");
    url.setHost(address.toString());
    url.setPort(port);

    m_serverUrl = url;

    qCDebug(dcRemoteProxyClientWebSocket()) << "Connecting to" << serverUrl().toString();
    m_webSocket->open(m_serverUrl);
}

void WebSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnecting from" << serverUrl().toString();
    m_webSocket->close();
}

}
