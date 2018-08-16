#ifndef REMOTEPROXYCONNECTOR_H
#define REMOTEPROXYCONNECTOR_H

#include <QUuid>
#include <QDebug>
#include <QObject>
#include <QHostInfo>
#include <QWebSocket>
#include <QHostAddress>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientConnection)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientConnectionTraffic)

namespace remoteproxyclient {

class JsonRpcClient;
class ProxyConnection;

class RemoteProxyConnection : public QObject
{
    Q_OBJECT
public:
    enum ConnectionType {
        ConnectionTypeWebSocket,
        ConnectionTypeTcpSocket
    };
    Q_ENUM(ConnectionType)

    enum State {
        StateHostLookup,
        StateConnecting,
        StateConnected,
        StateInitializing,
        StateReady,
        StateAuthenticating,
        StateWaitTunnel,
        StateRemoteConnected,
        StateDisconnected
    };
    Q_ENUM(State)

    enum Error {
        ErrorNoError,
        ErrorHostNotFound,
        ErrorSocketError,
        ErrorSslError,
        ErrorProxyNotResponding,
        ErrorProxyAuthenticationFailed
    };
    Q_ENUM(Error)

    explicit RemoteProxyConnection(const QUuid &clientUuid, const QString &clientName, QObject *parent = nullptr);
    ~RemoteProxyConnection();

    RemoteProxyConnection::State state() const;
    RemoteProxyConnection::Error error() const;
    QString errorString() const;

    void ignoreSslErrors();
    void ignoreSslErrors(const QList<QSslError> &errors);

    bool isConnected() const;
    bool isAuthenticated() const;
    bool isRemoteConnected() const;

    RemoteProxyConnection::ConnectionType connectionType() const;

    QUrl serverUrl() const;

    QString serverName() const;
    QString proxyServerName() const;
    QString proxyServerVersion() const;
    QString proxyServerApiVersion() const;

    QString tunnelPartnerName() const;
    QString tunnelPartnerUuid() const;

    bool sendData(const QByteArray &data);

private:
    ConnectionType m_connectionType = ConnectionTypeWebSocket;
    QUuid m_clientUuid;
    QString m_clientName;

    QUrl m_serverUrl;

    State m_state = StateDisconnected;
    Error m_error = ErrorNoError;

    bool m_insecureConnection = false;
    bool m_remoteConnected = false;

    JsonRpcClient *m_jsonClient = nullptr;
    ProxyConnection *m_connection = nullptr;

    // Server information
    QString m_serverName;
    QString m_proxyServerName;
    QString m_proxyServerVersion;
    QString m_proxyServerApiVersion;

    // Tunnel
    QString m_tunnelPartnerName;
    QString m_tunnelPartnerUuid;

    void cleanUp();

    void setState(State state);
    void setError(Error error);

signals:
    void connected();
    void ready();
    void authenticated();
    void remoteConnectionEstablished();
    void disconnected();

    void stateChanged(RemoteProxyConnection::State state);
    void errorOccured(RemoteProxyConnection::Error error);
    void sslErrors(const QList<QSslError> &errors);

    void dataReady(const QByteArray &data);

private slots:
    void onConnectionChanged(bool isConnected);
    void onConnectionDataAvailable(const QByteArray &data);
    void onConnectionSocketError();

    void onHelloFinished();
    void onAuthenticateFinished();
    void onTunnelEstablished(const QString &clientName, const QString &clientUuid);

public slots:
    bool connectServer(const QUrl &url);
    bool authenticate(const QString &token);
    void disconnectServer();

};

}

Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::Error);
Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::State);
Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::ConnectionType);

#endif // REMOTEPROXYCONNECTOR_H
