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

#ifndef TUNNELPROXYSERVER_H
#define TUNNELPROXYSERVER_H

#include <QUuid>
#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyTunnelProxyServer)

#include "proxyconnection.h"
#include "tunnelproxysocket.h"

namespace remoteproxyclient {

class ProxyConnection;
class TunnelProxyJsonRpcClient;

class TunnelProxyServer : public QObject
{
    Q_OBJECT
public:
    enum ConnectionType {
        ConnectionTypeWebSocket,
        ConnectionTypeTcpSocket
    };
    Q_ENUM(ConnectionType)

    explicit TunnelProxyServer(const QUuid &serverUuid, const QString &serverName, QObject *parent = nullptr);
    explicit TunnelProxyServer(const QUuid &serverUuid, const QString &serverName, ConnectionType connectionType, QObject *parent = nullptr);
    ~TunnelProxyServer();

    bool running() const;

    QAbstractSocket::SocketError error() const;

    void ignoreSslErrors();
    void ignoreSslErrors(const QList<QSslError> &errors);

public slots:
    void startServer(const QUrl &serverUrl);
    void stopServer();

signals:
    void runningChanged(bool running);
    void sslErrors(const QList<QSslError> &errors);

private:
    QUuid m_serverUuid;
    QString m_serverName;
    ConnectionType m_connectionType = ConnectionTypeTcpSocket;

    bool m_running = false;
    QUrl m_serverUrl;
    QAbstractSocket::SocketError m_error = QAbstractSocket::UnknownSocketError;

    ProxyConnection *m_connection = nullptr;
    TunnelProxyJsonRpcClient *m_jsonClient = nullptr;

};

}

#endif // TUNNELPROXYSERVER_H
