/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2023, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QUuid>
#include <QObject>
#include <QVariantMap>
#include <QLoggingCategory>

#include "jsonreply.h"
#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic)

namespace remoteproxyclient {

class JsonRpcClient : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcClient(ProxyConnection *connection, QObject *parent = nullptr);

    // General
    JsonReply *callHello();

    // Tunnel proxy
    JsonReply *callRegisterServer(const QUuid &serverUuid, const QString &serverName);
    JsonReply *callRegisterClient(const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid);
    JsonReply *callDisconnectClient(quint16 socketAddress);
    JsonReply *callPing(uint timestamp);

private:
    ProxyConnection *m_connection = nullptr;

    int m_commandId = 0;
    QByteArray m_dataBuffer;

    QHash<int, JsonReply *> m_replies;

    void sendRequest(const QVariantMap &request, bool slipEnabled = false);
    void processDataPacket(const QByteArray &data);

signals:
    void tunnelEstablished(const QString clientName, const QString &clientUuid);
    void tunnelProxyClientConnected(const QString &clientName, const QUuid &clientUuid, const QString &clientPeerAddress, quint16 socketAddress);
    void tunnelProxyClientDisonnected(quint16 socketAddress);

public slots:
    void processData(const QByteArray &data);

};

}

#endif // JSONRPCCLIENT_H
