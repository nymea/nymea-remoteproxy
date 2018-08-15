#ifndef ENGINE_H
#define ENGINE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>
#include <QSslConfiguration>

#include "proxyserver.h"
#include "monitorserver.h"
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

    void start(ProxyConfiguration *configuration);
    void stop();

    bool running() const;
    bool developerMode() const;

    QString serverName() const;

    void setAuthenticator(Authenticator *authenticator);
    void setDeveloperModeEnabled(bool enabled);

    Authenticator *authenticator() const;
    ProxyServer *proxyServer() const;
    WebSocketServer *webSocketServer() const;

private:
    explicit Engine(QObject *parent = nullptr);
    ~Engine();
    static Engine *s_instance;

    bool m_running = false;
    bool m_developerMode = false;

    ProxyConfiguration *m_configuration = nullptr;
    Authenticator *m_authenticator = nullptr;
    ProxyServer *m_proxyServer = nullptr;
    WebSocketServer *m_webSocketServer = nullptr;
    MonitorServer *m_monitorServer = nullptr;

signals:
    void runningChanged(bool running);

private slots:
    void clean();
    void setRunning(bool running);

};

}

#endif // ENGINE_H
