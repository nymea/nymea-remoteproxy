#ifndef ENGINE_H
#define ENGINE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>
#include <QSslConfiguration>

#include "proxyserver.h"
#include "websocketserver.h"
#include "proxyconfiguration.h"
#include "authentication/authenticator.h"

namespace remoteproxy {

class Engine : public QObject
{
    Q_OBJECT
public:
    static Engine *instance();
    void destroy();

    static bool exists();

    void start();
    void stop();

    bool running() const;

    QString serverName() const;
    void setServerName(const QString &serverName);

    void setWebSocketServerHostAddress(const QHostAddress &hostAddress);
    void setWebSocketServerPort(const quint16 &port);
    void setSslConfiguration(const QSslConfiguration &configuration);
    void setAuthenticationServerUrl(const QUrl &url);

    void setAuthenticator(Authenticator *authenticator);

    Authenticator *authenticator() const;
    ProxyServer *proxyServer() const;
    WebSocketServer *webSocketServer() const;

private:
    explicit Engine(QObject *parent = nullptr);
    ~Engine();
    static Engine *s_instance;

    bool m_running = false;
    QString m_serverName;

    quint16 m_webSocketServerPort = 1212;
    QHostAddress m_webSocketServerHostAddress = QHostAddress::LocalHost;
    QSslConfiguration m_sslConfiguration;
    QUrl m_authenticationServerUrl;

    ProxyConfiguration *m_configuration = nullptr;
    Authenticator *m_authenticator = nullptr;
    ProxyServer *m_proxyServer = nullptr;
    WebSocketServer *m_webSocketServer = nullptr;

signals:
    void runningChanged(bool running);

private slots:
    void clean();
    void setRunning(bool running);

};

}

#endif // ENGINE_H
