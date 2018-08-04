#include "remoteproxyconnector.h"

Q_LOGGING_CATEGORY(dcRemoteProxyConnector, "RemoteProxyConnector")

RemoteProxyConnector::RemoteProxyConnector(QObject *parent) : QObject(parent)
{

}

RemoteProxyConnector::~RemoteProxyConnector()
{
    disconnectServer();
}

RemoteProxyConnector::State RemoteProxyConnector::state() const
{
    return m_state;
}

RemoteProxyConnector::Error RemoteProxyConnector::error() const
{
    return m_error;
}

QString RemoteProxyConnector::errorString() const
{
    QString errorString;
    switch (m_error) {
    case ErrorNoError:
        errorString = "";
        break;
    case ErrorSocketError:
        errorString = "Socket connection error occured: " + socketErrorString();
        break;
    case ErrorSslError:
        errorString = "Socket SSL error occured.";
        break;
    case ErrorProxyNotResponding:
        errorString = "The proxy server does not respond.";
        break;
    case ErrorProxyAuthenticationFailed:
        errorString = "The authentication on the proxy server failed.";
        break;
    }

    return errorString;
}

QAbstractSocket::SocketError RemoteProxyConnector::socketError() const
{
    if (!m_webSocket)
        return QAbstractSocket::UnknownSocketError;

    return m_webSocket->error();
}

QString RemoteProxyConnector::socketErrorString() const
{
    if (!m_webSocket)
        return QString();

    return m_webSocket->errorString();
}

QUrl RemoteProxyConnector::serverUrl() const
{
    QUrl serverUrl;
    serverUrl.setScheme("wss");
    serverUrl.setHost(m_serverAddress.toString());
    serverUrl.setPort(m_serverPort);
    return serverUrl;
}

bool RemoteProxyConnector::isConnected() const
{
    return m_state == StateConnected || m_state == StateAuthenticating || m_state == StateWaitTunnel || m_state == StateTunnelEstablished;
}

bool RemoteProxyConnector::tunnelEstablished() const
{
    return m_state == StateTunnelEstablished;
}

RemoteProxyConnector::ConnectionType RemoteProxyConnector::connectionType() const
{
    return m_connectionType;
}

QHostAddress RemoteProxyConnector::serverAddress() const
{
    return m_serverAddress;
}

quint16 RemoteProxyConnector::serverPort() const
{
    return m_serverPort;
}

bool RemoteProxyConnector::insecureConnection() const
{
    return m_insecureConnection;
}

void RemoteProxyConnector::setInsecureConnection(bool insecureConnection)
{
    m_insecureConnection = insecureConnection;
}

bool RemoteProxyConnector::sendData(const QByteArray &data)
{
    // FIXME: reenable once the auth process is finished
//    if (m_state != StateTunnelEstablished) {
//        qWarning() << "RemoteProxyClient: There is no established tunnel for" << serverUrl().toString() << "yet.";
//        return false;
//    }

    if (!m_webSocket) {
        qCWarning(dcRemoteProxyConnector()) << "There is no websocket";
        return false;
    }

    qint64 dataSendCount = m_webSocket->sendTextMessage(QString::fromUtf8(data));
    if (dataSendCount != data.count()) {
        qCWarning(dcRemoteProxyConnector()) << "Could not send all data to" << serverUrl().toString();
        return false;
    }

    return true;
}

void RemoteProxyConnector::setState(RemoteProxyConnector::State state)
{
    if (m_state == state)
        return;

    qCDebug(dcRemoteProxyConnector()) << "State changed" << state;
    m_state = state;
    emit stateChanged(m_state);
}

void RemoteProxyConnector::setError(RemoteProxyConnector::Error error)
{
    if (m_error == error)
        return;

    qCDebug(dcRemoteProxyConnector()) << "Error occured" << error;
    m_error = error;
    emit errorOccured(m_error);
}


void RemoteProxyConnector::setConnectionType(RemoteProxyConnector::ConnectionType type)
{
    m_connectionType = type;
}

void RemoteProxyConnector::setServerAddress(const QHostAddress serverAddress)
{
    m_serverAddress = serverAddress;
}

void RemoteProxyConnector::setServerPort(quint16 serverPort)
{
    m_serverPort = serverPort;
}

void RemoteProxyConnector::onSocketConnected()
{
    setState(StateConnected);
    qCDebug(dcRemoteProxyConnector()) << "Connected to" << serverUrl().toString();
    emit connected();

    // TODO: start authentication process

    setState(StateAuthenticating);
}

void RemoteProxyConnector::onSocketDisconnected()
{
    qCDebug(dcRemoteProxyConnector()) << "Disconnected from" << serverUrl().toString();
    setState(StateDisconnected);
    emit disconnected();
}

void RemoteProxyConnector::onSocketError(QAbstractSocket::SocketError error)
{
    qCWarning(dcRemoteProxyConnector()) << "Socket error occured" << error;
    setError(ErrorSocketError);
}

void RemoteProxyConnector::onSocketSslError(const QList<QSslError> &errors)
{
    if (m_insecureConnection) {
        qCDebug(dcRemoteProxyConnector()) << "Ignore ssl errors because explicit allowed to use an insecure connection.";
        m_webSocket->ignoreSslErrors();
    } else {
        qCWarning(dcRemoteProxyConnector()) << "Socket ssl errors occured:";
        foreach (const QSslError sslError, errors) {
            qCWarning(dcRemoteProxyConnector()) << "    -->" << static_cast<int>(sslError.error()) << sslError.errorString();
        }
        setError(ErrorSslError);
    }
}

void RemoteProxyConnector::onSocketStateChanged(QAbstractSocket::SocketState state)
{
    qCDebug(dcRemoteProxyConnector()) << "Socket state changed" << state;
    switch (state) {
    case QAbstractSocket::ConnectingState:
    case QAbstractSocket::HostLookupState:
        setState(StateConnecting);
        break;
    case QAbstractSocket::ConnectedState:
        setState(StateConnected);
        break;
    default:
        setState(StateDisconnected);
        break;
    }
}

void RemoteProxyConnector::onTextMessageReceived(const QString &message)
{
    // TODO: check if tunnel is established, if so, emit data received
    qCDebug(dcRemoteProxyConnector()) << "Data received" << message;
}

void RemoteProxyConnector::onBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message);
}

bool RemoteProxyConnector::connectServer(const QHostAddress &serverAddress, quint16 port)
{
    setServerAddress(serverAddress);
    setServerPort(port);

    switch (m_connectionType) {
    // TODO: currently only websocket support
    case ConnectionTypeWebSocket:
        disconnectServer();

        m_webSocket = new QWebSocket("libnymea-remoteproxyclient", QWebSocketProtocol::VersionLatest, this);

        if (m_insecureConnection)
            m_webSocket->ignoreSslErrors();

        connect(m_webSocket, &QWebSocket::connected, this, &RemoteProxyConnector::onSocketConnected);
        connect(m_webSocket, &QWebSocket::disconnected, this, &RemoteProxyConnector::onSocketDisconnected);
        connect(m_webSocket, &QWebSocket::textMessageReceived, this, &RemoteProxyConnector::onTextMessageReceived);
        connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &RemoteProxyConnector::onBinaryMessageReceived);

        connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
        connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSocketSslError(QList<QSslError>)));
        connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));

        setState(StateConnecting);
        m_webSocket->open(serverUrl());
        qCDebug(dcRemoteProxyConnector()) << "Start connecting to" << serverUrl().toString();
        return true;
    }

    return false;
}

void RemoteProxyConnector::disconnectServer()
{
    if (!m_webSocket)
        return;

    qCDebug(dcRemoteProxyConnector()) << "Disconnect from server" << serverUrl().toString();
    m_webSocket->close(QWebSocketProtocol::CloseCodeNormal, "Bye bye");
    m_webSocket->deleteLater();
    m_webSocket = nullptr;
    setState(StateDisconnected);
}
