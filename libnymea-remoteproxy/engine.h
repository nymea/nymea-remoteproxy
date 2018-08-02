#ifndef ENGINE_H
#define ENGINE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>
#include <QSslConfiguration>

#include "websocketserver.h"

class Engine : public QObject
{
    Q_OBJECT
public:
    static Engine *instance();
    static bool exists();
    void destroy();

    void start();
    void stop();

    bool running() const;

    void setWebSocketServerHostAddress(const QHostAddress &hostAddress);
    void setWebSocketServerPort(const quint16 &port);

    void setSslConfiguration(const QSslConfiguration &configuration);
    void setAuthenticationServerUrl(const QUrl &url);

    WebSocketServer *webSocketServer() const;

private:
    explicit Engine(QObject *parent = nullptr);
    ~Engine();
    static Engine *s_instance;

    bool m_running = false;

    quint16 m_webSocketServerPort = 1212;
    QHostAddress m_webSocketServerHostAddress = QHostAddress::LocalHost;

    QSslConfiguration m_sslConfiguration;
    QUrl m_authenticationServerUrl;

    WebSocketServer *m_webSocketServer = nullptr;

    void setRunning(bool running);

signals:
    void runningChanged(bool running);

};

#endif // ENGINE_H
