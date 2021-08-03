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

#ifndef TUNNELPROXYSERVERCONNECTION_H
#define TUNNELPROXYSERVERCONNECTION_H

#include <QUuid>
#include <QObject>
#include <QDebug>

namespace remoteproxy {

class TransportClient;
class TunnelProxyClientConnection;

class TunnelProxyServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit TunnelProxyServerConnection(TransportClient *transportClient, const QUuid &serverUuid, const QString &serverName, QObject *parent = nullptr);

    TransportClient *transportClient() const;

    QUuid serverUuid() const;
    QString serverName() const;

    QList<TunnelProxyClientConnection *> clientConnections() const;

    void registerClientConnection(TunnelProxyClientConnection *clientConnection);
    void unregisterClientConnection(TunnelProxyClientConnection *clientConnection);

    TunnelProxyClientConnection *getClientConnection(quint16 socketAddress);

signals:

private:
    TransportClient *m_transportClient = nullptr;
    QUuid m_serverUuid;
    QString m_serverName;
    quint16 m_connectionLimit = 100;

    quint16 m_currentAddressCounter = 0;

    QHash<QUuid, TunnelProxyClientConnection *> m_clientConnections;
    QHash<quint16, TunnelProxyClientConnection *> m_clientConnectionsAddresses;

    quint16 getFreeAddress();

};

QDebug operator<<(QDebug debug, TunnelProxyServerConnection *serverConnection);

}

#endif // TUNNELPROXYSERVERCONNECTION_H
