#ifndef REMOTEPROXYCONNECTOR_H
#define REMOTEPROXYCONNECTOR_H

#include <QDebug>
#include <QObject>
#include <QWebSocket>
#include <QHostAddress>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyConnector)

class RemoteProxyConnector : public QObject
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
        StateTunnelEstablished,
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

    explicit RemoteProxyConnector(QObject *parent = nullptr);
    ~RemoteProxyConnector();

    State state() const;

    Error error() const;
    QString errorString() const;

    QAbstractSocket::SocketError socketError() const;
    QString socketErrorString() const;

    QUrl serverUrl() const;

    bool isConnected() const;
    bool tunnelEstablished() const;

    ConnectionType connectionType() const;
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
    bool m_tunnelEstablished = false;
    QWebSocket *m_webSocket = nullptr;

    void setState(State state);
    void setError(Error error);

    void setConnectionType(ConnectionType type);
    void setServerAddress(const QHostAddress serverAddress);
    void setServerPort(quint16 serverPort);

signals:
    void connected();
    void disconnected();
    void tunnelEstablished();
    void stateChanged(RemoteProxyConnector::State state);
    void errorOccured(RemoteProxyConnector::Error error);

    void dataReady(const QByteArray &data);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketSslError(const QList<QSslError> &errors);
    void onSocketStateChanged(QAbstractSocket::SocketState state);
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);


public slots:
    bool connectServer(const QHostAddress &serverAddress, quint16 port);
    void disconnectServer();

};

Q_DECLARE_METATYPE(RemoteProxyConnector::State);
Q_DECLARE_METATYPE(RemoteProxyConnector::Error);
Q_DECLARE_METATYPE(RemoteProxyConnector::ConnectionType);

#endif // REMOTEPROXYCONNECTOR_H
