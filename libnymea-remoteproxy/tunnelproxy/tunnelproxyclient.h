#ifndef TUNNELPROXYCLIENT_H
#define TUNNELPROXYCLIENT_H

#include <QObject>
#include <QTimer>

#include "server/transportclient.h"

namespace remoteproxy {

class TunnelProxyClient : public TransportClient
{
    Q_OBJECT
public:
    enum Type {
        TypeNone,
        TypeServer,
        TypeClient
    };
    Q_ENUM(Type)

    explicit TunnelProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent = nullptr);

    Type type() const;
    void setType(Type type);

    // Json server methods
    QList<QByteArray> processData(const QByteArray &data) override;

    // This method will be called from the proxy server once the client is
    // registered correctly as server or client connection and is now active
    void activateClient();

signals:
    void typeChanged(Type type);

private:
    QTimer *m_inactiveTimer = nullptr;
    Type m_type = TypeNone;

};

QDebug operator<< (QDebug debug, TunnelProxyClient *tunnelProxyClient);

}

#endif // TUNNELPROXYCLIENT_H
