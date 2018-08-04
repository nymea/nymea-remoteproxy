#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <QUuid>
#include <QObject>

#include "proxyclient.h"
#include "transportinterface.h"

class ProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit ProxyServer(QObject *parent = nullptr);

    void registerTransportInterface(TransportInterface *interface);

private:
    QList<TransportInterface *> m_transportInterfaces;
    QList<QUuid> m_unauthenticatedClients;

private slots:
    void onClientConnected(const QUuid &clientId);
    void onClientDisconnected(const QUuid &clientId);
    void onClientDataAvailable(const QUuid &clientId, const QByteArray &data);

public slots:
    void startServer();
    void stopServer();

};

#endif // PROXYSERVER_H
