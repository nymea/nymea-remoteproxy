#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <QUuid>
#include <QHash>
#include <QObject>

#include "proxyclient.h"
#include "jsonrpcserver.h"
#include "transportinterface.h"

namespace remoteproxy {

class ProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit ProxyServer(QObject *parent = nullptr);
    ~ProxyServer();

    void registerTransportInterface(TransportInterface *interface);

private:
    JsonRpcServer *m_jsonRpcServer = nullptr;
    QList<TransportInterface *> m_transportInterfaces;

    QHash<QUuid, ProxyClient *> m_proxyClients;
    QHash<ProxyClient *, ProxyClient *> m_tunnels;

    void sendResponse(TransportInterface *interface, const QUuid &clientId, const QVariantMap &response = QVariantMap());

private slots:
    void onClientConnected(const QUuid &clientId);
    void onClientDisconnected(const QUuid &clientId);
    void onClientDataAvailable(const QUuid &clientId, const QByteArray &data);

public slots:
    void startServer();
    void stopServer();

};

}

#endif // PROXYSERVER_H
