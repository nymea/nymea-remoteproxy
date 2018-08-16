#ifndef WEBSOCKETCONNECTOR_H
#define WEBSOCKETCONNECTOR_H

#include <QDebug>
#include <QObject>
#include <QWebSocket>
#include <QLoggingCategory>

#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientWebSocket)

namespace remoteproxyclient {

class WebSocketConnection : public ProxyConnection
{
    Q_OBJECT
public:
    explicit WebSocketConnection(QObject *parent = nullptr);
    ~WebSocketConnection() override;

    QUrl serverUrl() const override;

    void sendData(const QByteArray &data) override;

    void ignoreSslErrors() override;
    void ignoreSslErrors(const QList<QSslError> &errors) override;

private:
    QUrl m_serverUrl;
    QWebSocket *m_webSocket = nullptr;

private slots:
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);

public slots:
    void connectServer(const QUrl &serverUrl) override;
    void disconnectServer() override;
};

}

#endif // WEBSOCKETCONNECTOR_H
