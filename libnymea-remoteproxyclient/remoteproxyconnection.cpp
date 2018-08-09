#include "remoteproxyconnection.h"
#include "websocketconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientConnection, "RemoteProxyClientConnection")
Q_LOGGING_CATEGORY(dcRemoteProxyClientConnectionTraffic, "RemoteProxyClientConnectionTraffic")

namespace remoteproxyclient {

RemoteProxyConnection::RemoteProxyConnection(ConnectionType connectionType, QObject *parent) :
    QObject(parent),
    m_connectionType(connectionType)
{

}

RemoteProxyConnection::~RemoteProxyConnection()
{
    cleanUp();
}

RemoteProxyConnection::State RemoteProxyConnection::state() const
{
    return m_state;
}

RemoteProxyConnection::Error RemoteProxyConnection::error() const
{
    return m_error;
}

QString RemoteProxyConnection::errorString() const
{
    QString errorString;
    switch (m_error) {
    case ErrorNoError:
        errorString = "";
        break;
    case ErrorSocketError:
        errorString = "Socket connection error occured.";
        break;
    case ErrorSslError:
        errorString = "SSL error occured.";
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

bool RemoteProxyConnection::isConnected() const
{
    return m_state == StateConnected || m_state == StateAuthenticating || m_state == StateWaitTunnel || m_state == StateRemoteConnected;
}

bool RemoteProxyConnection::isRemoteConnected() const
{
    return m_state == StateRemoteConnected;
}

RemoteProxyConnection::ConnectionType RemoteProxyConnection::connectionType() const
{
    return m_connectionType;
}

QHostAddress RemoteProxyConnection::serverAddress() const
{
    return m_serverAddress;
}

quint16 RemoteProxyConnection::serverPort() const
{
    return m_serverPort;
}

bool RemoteProxyConnection::insecureConnection() const
{
    return m_insecureConnection;
}

void RemoteProxyConnection::setInsecureConnection(bool insecureConnection)
{
    m_insecureConnection = insecureConnection;
}

bool RemoteProxyConnection::sendData(const QByteArray &data)
{
    if (!isConnected()) {
        qCWarning(dcRemoteProxyClientConnection()) << "Could not send data. Not connected.";
        return false;
    }

    if (!isRemoteConnected()) {
        qCWarning(dcRemoteProxyClientConnection()) << "Could not send data. The remote client is not connected yet.";
        return false;
    }

    m_connection->sendData(data);
    return true;
}

void RemoteProxyConnection::cleanUp()
{
    if (m_jsonClient) {
        delete m_jsonClient;
        m_jsonClient = nullptr;
    }

    if (m_connection) {
        delete m_connection;
        m_connection = nullptr;
    }

    setState(StateDisconnected);
}

void RemoteProxyConnection::setState(RemoteProxyConnection::State state)
{
    if (m_state == state)
        return;

    qCDebug(dcRemoteProxyClientConnection()) << "State changed" << state;
    m_state = state;
    emit stateChanged(m_state);
}

void RemoteProxyConnection::setError(RemoteProxyConnection::Error error)
{
    if (m_error == error)
        return;

    qCDebug(dcRemoteProxyClientConnection()) << "Error occured" << error;
    m_error = error;
    emit errorOccured(m_error);
}

void RemoteProxyConnection::onConnectionChanged(bool isConnected)
{
    if (!isConnected) {
        emit disconnected();
        cleanUp();
    } else {
        setState(StateConnected);
        emit connected();

        JsonReply *reply = m_jsonClient->callHello();
        connect(reply, &JsonReply::finished, this, &RemoteProxyConnection::onHelloFinished);
    }
}

void RemoteProxyConnection::onConnectionDataAvailable(const QByteArray &data)
{
    switch (m_state) {
    case StateConnected:
        m_jsonClient->processData(data);
    case StateRemoteConnected:
        // Remote data arrived
        emit dataReady(data);
    default:
        qCDebug(dcRemoteProxyClientConnection()) << "Unhandled: Data reviced" << data;
    }
}

void RemoteProxyConnection::onConnectionSocketError()
{
    setError(ErrorSocketError);
}

void RemoteProxyConnection::onConnectionSslError()
{
    setError(ErrorSslError);
}

void RemoteProxyConnection::onHelloFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    QVariantMap response = reply->response();
    qCDebug(dcRemoteProxyClientConnection()) << "Hello response ready" << response;
}

void RemoteProxyConnection::onAuthenticateFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    QVariantMap response = reply->response();
    qCDebug(dcRemoteProxyClientConnection()) << "Hello response ready" << response;
}

bool RemoteProxyConnection::connectServer(const QHostAddress &serverAddress, quint16 port)
{    
    m_serverAddress = serverAddress;
    m_serverPort = port;

    if (m_connection) {
        delete m_connection;
        m_connection = nullptr;
    }

    if (m_jsonClient) {
        delete m_jsonClient;
        m_jsonClient = nullptr;
    }

    switch (m_connectionType) {
    case ConnectionTypeWebSocket:
        m_connection = qobject_cast<ProxyConnection *>(new WebSocketConnection(this));
    }

    m_connection->setAllowSslErrors(m_insecureConnection);

    connect(m_connection, &ProxyConnection::connectedChanged, this, &RemoteProxyConnection::onConnectionChanged);
    connect(m_connection, &ProxyConnection::dataReceived, this, &RemoteProxyConnection::onConnectionDataAvailable);
    connect(m_connection, &ProxyConnection::errorOccured, this, &RemoteProxyConnection::onConnectionSocketError);
    connect(m_connection, &ProxyConnection::sslErrorOccured, this, &RemoteProxyConnection::onConnectionSslError);

    m_jsonClient = new JsonRpcClient(this);

    qCDebug(dcRemoteProxyClientConnection()) << "Connecting to" << QString("%1:%2").arg(serverAddress.toString()).arg(port);
    m_connection->connectServer(serverAddress, port);

    setState(StateConnecting);

    return true;
}

void RemoteProxyConnection::disconnectServer()
{
    if (m_connection)
        qCDebug(dcRemoteProxyClientConnection()) << "Disconnect from" << m_connection->serverUrl().toString();

    cleanUp();
}

}
