#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <QUuid>
#include <QHash>
#include <QObject>

#include "proxyclient.h"
#include "jsonrpcserver.h"
#include "tunnelconnection.h"
#include "transportinterface.h"

namespace remoteproxy {

class ProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit ProxyServer(QObject *parent = nullptr);
    ~ProxyServer();

    bool running() const;
    void registerTransportInterface(TransportInterface *interface);

private:
    JsonRpcServer *m_jsonRpcServer = nullptr;
    QList<TransportInterface *> m_transportInterfaces;

    bool m_running = false;

    // Transport ClientId, ProxyClient
    QHash<QUuid, ProxyClient *> m_proxyClients;

    // Token, ProxyClient
    QHash<QString, ProxyClient *> m_authenticatedClients;

    // Token, Tunnel
    QHash<QString, TunnelConnection> m_tunnels;

    void setRunning(bool running);

    ProxyClient *getRemoteClient(ProxyClient *proxyClient);

    void sendResponse(TransportInterface *interface, const QUuid &clientId, const QVariantMap &response = QVariantMap());
    void establishTunnel(ProxyClient *firstClient, ProxyClient *secondClient);

signals:
    void runningChanged();

private slots:
    void onClientConnected(const QUuid &clientId);
    void onClientDisconnected(const QUuid &clientId);
    void onClientDataAvailable(const QUuid &clientId, const QByteArray &data);

    void onProxyClientAuthenticated();
    void onProxyClientTunnelConnected();

public slots:
    void startServer();
    void stopServer();

};

}

#endif // PROXYSERVER_H
