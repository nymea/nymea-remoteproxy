#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QUuid>
#include <QDebug>
#include <QObject>

#include "transportinterface.h"

namespace remoteproxy {

class ProxyClient : public QObject
{
    Q_OBJECT

public:
    explicit ProxyClient(TransportInterface *interface, const QUuid &clientId, QObject *parent = nullptr);

    QUuid clientId() const;

    bool authenticated() const;
    void setAuthenticated(bool authenticated);

    bool tunnelConnected() const;
    void setTunnelConnected(bool tunnelConnected);

    TransportInterface *interface() const;

    // Properties from auth request
    QString uuid() const;
    void setUuid(const QString &uuid);

    QString name() const;
    void setName(const QString &name);

    QString token() const;
    void setToken(const QString &token);


private:
    TransportInterface *m_interface = nullptr;
    QUuid m_clientId;
    bool m_authenticated = false;
    bool m_tunnelConnected = false;
    QString m_uuid;
    QString m_name;
    QString m_token;

signals:
    void authenticatedChanged(bool authenticated);
    void tunnelConnectedChanged(bool tunnelConnected);

};

QDebug operator<< (QDebug debug, ProxyClient *proxyClient);

}

#endif // PROXYCLIENT_H
