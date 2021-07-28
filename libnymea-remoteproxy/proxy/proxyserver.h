/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <QUuid>
#include <QHash>
#include <QObject>

#include "proxyclient.h"
#include "tunnelconnection.h"
#include "server/jsonrpcserver.h"
#include "server/transportinterface.h"

namespace remoteproxy {

class ProxyServer : public QObject
{
    Q_OBJECT
public:
    explicit ProxyServer(QObject *parent = nullptr);
    ~ProxyServer();

    bool running() const;
    void setRunning(bool running);

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

    // TunnelIdentifier (token + nonce), ProxyClient
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
