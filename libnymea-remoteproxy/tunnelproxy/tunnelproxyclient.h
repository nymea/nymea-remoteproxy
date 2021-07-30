#ifndef TUNNELPROXYCLIENT_H
#define TUNNELPROXYCLIENT_H

#include <QObject>

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

    bool slipEnabled() const;
    void setSlipEnabled(bool slipEnabled);

    // Json server methods
    QList<QByteArray> processData(const QByteArray &data) override;

signals:
    void typeChanged(Type type);

private:
    Type m_type = TypeNone;
    bool m_slipEnabled = false;

};

QDebug operator<< (QDebug debug, TunnelProxyClient *tunnelProxyClient);

}

#endif // TUNNELPROXYCLIENT_H
