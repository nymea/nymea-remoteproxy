#include "websocketconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientWebSocket, "RemoteProxyClientWebSocket")

namespace remoteproxyclient {

WebSocketConnection::WebSocketConnection(QObject *parent) :
    ProxyConnection(parent)
{
    m_webSocket = new QWebSocket("libnymea-remoteproxyclient", QWebSocketProtocol::Version13, this);

    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketConnection::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketConnection::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketConnection::onBinaryMessageReceived);

    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SIGNAL(sslErrors(QList<QSslError>)));
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

void WebSocketConnection::ignoreSslErrors()
{
    m_webSocket->ignoreSslErrors();
}

void WebSocketConnection::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_webSocket->ignoreSslErrors(errors);
}

void WebSocketConnection::onDisconnected()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnected from" << m_webSocket->requestUrl().toString() << m_webSocket->closeReason();
    emit connectedChanged(false);
}

void WebSocketConnection::onError(QAbstractSocket::SocketError error)
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket error occured" << error << m_webSocket->errorString();
    emit errorOccured();
}

void WebSocketConnection::onStateChanged(QAbstractSocket::SocketState state)
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket state changed" << state;

    switch (state) {
    case QAbstractSocket::ConnectedState:
        qCDebug(dcRemoteProxyClientWebSocket()) << "Connected with" << m_webSocket->requestUrl().toString();
        setConnected(true);
        break;
    default:
        setConnected(false);
        break;
    }
}

void WebSocketConnection::onTextMessageReceived(const QString &message)
{
    emit dataReceived(message.toUtf8());
}

void WebSocketConnection::onBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message)
}

void WebSocketConnection::connectServer(const QUrl &serverUrl)
{
    if (connected()) {
        m_webSocket->close();
    }

    m_serverUrl = serverUrl;
    qCDebug(dcRemoteProxyClientWebSocket()) << "Connecting to" << m_serverUrl.toString();
    m_webSocket->open(m_serverUrl);
}

void WebSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnecting from" << serverUrl().toString();
    m_webSocket->close();
}

}
