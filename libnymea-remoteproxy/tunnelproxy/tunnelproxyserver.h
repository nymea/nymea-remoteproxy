/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2021, nymea GmbH
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

#ifndef TUNNELPROXYSERVER_H
#define TUNNELPROXYSERVER_H

#include <QObject>

#include "server/jsonrpcserver.h"
#include "server/transportinterface.h"
#include "tunnelproxyclient.h"

namespace remoteproxy {

class TunnelProxyServerConnection;
class TunnelProxyClientConnection;

class TunnelProxyServer : public QObject
{
    Q_OBJECT
public:
    enum TunnelProxyError {
        TunnelProxyErrorNoError,
        TunnelProxyErrorInvalidUuid,
        TunnelProxyErrorInternalServerError,
        TunnelProxyErrorServerNotFound,
        TunnelProxyErrorAlreadyRegistered
    };
    Q_ENUM(TunnelProxyError)

    explicit TunnelProxyServer(QObject *parent = nullptr);
    ~TunnelProxyServer();

    bool running() const;
    void setRunning(bool running);

    void registerTransportInterface(TransportInterface *interface);

    TunnelProxyServer::TunnelProxyError registerServer(const QUuid &clientId, const QUuid &serverUuid, const QString &serverName);
    TunnelProxyServer::TunnelProxyError registerClient(const QUuid &clientId, const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid);

public slots:
    void startServer();
    void stopServer();

    void tick();

signals:
    void runningChanged(bool running);

private slots:
    void onClientConnected(const QUuid &clientId, const QHostAddress &address);
    void onClientDisconnected(const QUuid &clientId);
    void onClientDataAvailable(const QUuid &clientId, const QByteArray &data);

private:
    JsonRpcServer *m_jsonRpcServer = nullptr;
    QList<TransportInterface *> m_transportInterfaces;

    bool m_running = false;

    QHash<QUuid, TunnelProxyClient *> m_proxyClients; // clientId, object

    // Server connections
    QHash<QUuid, TunnelProxyServerConnection *> m_tunnelProxyServerConnections; // server uuid, object
    QHash<QUuid, TunnelProxyClientConnection *> m_tunnelProxyClientConnections; // client uuid, object

};

}

#endif // TUNNELPROXYSERVER_H
