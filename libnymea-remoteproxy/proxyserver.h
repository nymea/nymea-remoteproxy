/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

    QVariantMap currentStatistics();

private:
    JsonRpcServer *m_jsonRpcServer = nullptr;
    QList<TransportInterface *> m_transportInterfaces;

    bool m_running = false;

    // Transport ClientId, ProxyClient
    QHash<QUuid, ProxyClient *> m_proxyClients;

    // FIXME: Token, ProxyClient
    QHash<QString, ProxyClient *> m_authenticatedClients;

    // Nonce, ProxyClient
    QHash<QString, ProxyClient *> m_authenticatedClientsNonce;

    // Token, Tunnel
    QHash<QString, TunnelConnection> m_tunnels;

    // Statistic measurments
    int m_troughput = 0;
    int m_troughputCounter = 0;

    // Persistent statistics
    int m_totalClientCount = 0;
    int m_totalTunnelCount = 0;
    int m_totalTraffic = 0;


    // Set private properties
    void setRunning(bool running);

    void loadStatistics();
    void saveStatistics();


    // Helper methods
    ProxyClient *getRemoteClient(ProxyClient *proxyClient);
    void establishTunnel(ProxyClient *firstClient, ProxyClient *secondClient);

signals:
    void runningChanged();

private slots:
    void onClientConnected(const QUuid &clientId, const QHostAddress &address);
    void onClientDisconnected(const QUuid &clientId);
    void onClientDataAvailable(const QUuid &clientId, const QByteArray &data);

    void onProxyClientAuthenticated();
    void onProxyClientTimeoutOccured();

public slots:
    void startServer();
    void stopServer();

    void tick();

};

}

#endif // PROXYSERVER_H
