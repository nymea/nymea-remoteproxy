#ifndef MONITORSERVER_H
#define MONITORSERVER_H

#include <QTimer>
#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

namespace remoteproxy {

class MonitorServer : public QObject
{
    Q_OBJECT
public:
    explicit MonitorServer(const QString &serverName, QObject *parent = nullptr);

private:
    QString m_serverName;
    QLocalServer *m_server = nullptr;
    QTimer *m_timer = nullptr;
    QList<QLocalSocket *> m_clients;

    QVariantMap createMonitorData();
    void sendMonitorData(QLocalSocket *clientConnection, const QVariantMap &dataMap);

private slots:
    void onTimeout();
    void onMonitorConnected();
    void onMonitorDisconnected();

public slots:
    void startServer();
    void stopServer();
};

}

#endif // MONITORSERVER_H
