// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TUNNELPROXYSOCKET_H
#define TUNNELPROXYSOCKET_H

#include <QUuid>
#include <QObject>
#include <QHostAddress>

namespace remoteproxyclient {

class ProxyConnection;
class TunnelProxySocketServer;

class TunnelProxySocket : public QObject
{
    Q_OBJECT
    friend class TunnelProxySocketServer;

public:
    QUuid clientUuid() const;
    QString clientName() const;
    QHostAddress clientPeerAddress() const;
    quint16 socketAddress() const;

    bool connected() const;

    void writeData(const QByteArray &data);

    void disconnectSocket();

signals:
    void dataReceived(const QByteArray &data);

    void connectedChanged(bool connected);
    void disconnected();

private:
    explicit TunnelProxySocket(ProxyConnection *connection, TunnelProxySocketServer *socketServer, const QString &clientName, const QUuid &clientUuid, const QHostAddress &clientPeerAddress, quint16 socketAddress, QObject *parent = nullptr);
    ~TunnelProxySocket() = default;

    ProxyConnection *m_connection = nullptr;
    TunnelProxySocketServer *m_socketServer = nullptr;
    bool m_connected = true; // Note: on creation, the socket is connected, otherwise it would not have been created

    QString m_clientName;
    QUuid m_clientUuid;
    QHostAddress m_clientPeerAddress;
    quint16 m_socketAddress = 0xFFFF;

    void setDisconnected();

};

QDebug operator<<(QDebug debug, TunnelProxySocket *tunnelProxySocket);

}

#endif // TUNNELPROXYSOCKET_H
