/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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
