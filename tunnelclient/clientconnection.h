#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QUrl>
#include <QObject>
#include <QTimer>

#include "tunnelproxy/tunnelproxyremoteconnection.h"

using namespace remoteproxyclient;

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, const QUuid &serverUuid, bool insecure, bool sendRandomData, QObject *parent = nullptr);

    void connectToServer();

private:
    QUrl m_serverUrl;
    QString m_name;
    QUuid m_uuid;
    QUuid m_serverUuid;
    bool m_insecure = false;
    bool m_sendRandomData = false;

    TunnelProxyRemoteConnection *m_remoteConnection = nullptr;
    QTimer m_timer;

    QString generateRandomString(uint length) const;
};

#endif // CLIENTCONNECTION_H
