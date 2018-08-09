#ifndef REMOTEPROXYCONNECTOR_H
#define REMOTEPROXYCONNECTOR_H

#include <QDebug>
#include <QObject>
#include <QWebSocket>
#include <QHostAddress>
#include <QLoggingCategory>

#include "jsonrpcclient.h"
#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientConnection)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientConnectionTraffic)

namespace remoteproxyclient {

class RemoteProxyConnection : public QObject
{
    Q_OBJECT
public:
    enum ConnectionType {
        ConnectionTypeWebSocket
    };
    Q_ENUM(ConnectionType)

    enum State {
        StateConnecting,
        StateConnected,
        StateAuthenticating,
        StateWaitTunnel,
        StateRemoteConnected,
        StateDisconnected
    };
    Q_ENUM(State)

    enum Error {
        ErrorNoError,
        ErrorSocketError,
        ErrorSslError,
        ErrorProxyNotResponding,
        ErrorProxyAuthenticationFailed
    };
    Q_ENUM(Error)

    explicit RemoteProxyConnection(ConnectionType connectionType = ConnectionTypeWebSocket, QObject *parent = nullptr);
    ~RemoteProxyConnection();

    RemoteProxyConnection::State state() const;

    RemoteProxyConnection::Error error() const;
    QString errorString() const;

    bool isConnected() const;
    bool isRemoteConnected() const;

    RemoteProxyConnection::ConnectionType connectionType() const;
    QHostAddress serverAddress() const;
    quint16 serverPort() const;

    bool insecureConnection() const;
    void setInsecureConnection(bool insecureConnection);

    bool sendData(const QByteArray &data);

private:
    ConnectionType m_connectionType = ConnectionTypeWebSocket;
    QHostAddress m_serverAddress;
    quint16 m_serverPort = 443;
    State m_state = StateDisconnected;
    Error m_error = ErrorNoError;
    bool m_insecureConnection = false;
    bool m_remoteConnected = false;

    JsonRpcClient *m_jsonClient = nullptr;
    ProxyConnection *m_connection = nullptr;

    void cleanUp();

    void setState(State state);
    void setError(Error error);

signals:
    void connected();
    void disconnected();
    void remoteConnectedChanged(bool remoteConnected);
    void stateChanged(RemoteProxyConnection::State state);
    void errorOccured(RemoteProxyConnection::Error error);

    void dataReady(const QByteArray &data);

private slots:
    void onConnectionChanged(bool isConnected);
    void onConnectionDataAvailable(const QByteArray &data);
    void onConnectionSocketError();
    void onConnectionSslError();

    void onHelloFinished();
    void onAuthenticateFinished();

public slots:
    bool connectServer(const QHostAddress &serverAddress, quint16 port);
    void disconnectServer();

};

}

Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::State);
Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::Error);
Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::ConnectionType);

#endif // REMOTEPROXYCONNECTOR_H
