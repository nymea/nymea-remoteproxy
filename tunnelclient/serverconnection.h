#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QUuid>
#include <QObject>

#include "tunnelproxy/tunnelproxysocketserver.h"

using namespace remoteproxyclient;

class ServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit ServerConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, bool insecure, bool echo, QObject *parent = nullptr);

    void startServer();

private:
    QUrl m_serverUrl;
    QString m_name;
    QUuid m_uuid;
    bool m_insecure = false;
    bool m_echo = false;

    TunnelProxySocketServer *m_socketServer = nullptr;
};

#endif // SERVERCONNECTION_H
