#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QUrl>
#include <QUuid>
#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QSslConfiguration>

#include "transportinterface.h"

class WebSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit WebSocketServer(const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~WebSocketServer() override;

    QUrl serverUrl() const;
    void setServerUrl(const QUrl &serverUrl);

    bool running() const;

    QSslConfiguration sslConfiguration() const;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

private:
    QUrl m_serverUrl;
    QWebSocketServer *m_server = nullptr;
    QSslConfiguration m_sslConfiguration;
    bool m_enabled = false;

    QHash<QUuid, QWebSocket *> m_clientList;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onBinaryMessageReceived(const QByteArray &data);
    void onTextMessageReceived(const QString &message);
    void onClientError(QAbstractSocket::SocketError error);
    void onServerError(QAbstractSocket::SocketError error);
    void onPing(quint64 elapsedTime, const QByteArray & payload);

public slots:
    bool startServer() override;
    bool stopServer() override;

};

#endif // WEBSOCKETSERVER_H
