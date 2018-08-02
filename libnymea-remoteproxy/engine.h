#ifndef ENGINE_H
#define ENGINE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>
#include <QSslConfiguration>

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

    void setHost(const QHostAddress &hostAddress);
    void setAuthenticationServerUrl(const QUrl &url);
    void setPort(const quint16 &port);
    void setSslConfiguration(const QSslConfiguration &configuration);

private:
    explicit Engine(QObject *parent = nullptr);
    static Engine *s_instance;

    bool m_running = false;

    quint16 m_port = 0;
    QHostAddress m_hostAddress;
    QSslConfiguration m_sslConfiguration;

    QUrl m_authenticationServerUrl;

    void setRunning(bool running);

signals:
    void runningChanged(bool running);

};

#endif // ENGINE_H
