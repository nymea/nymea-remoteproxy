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

#ifndef TUNNELPROXYSOCKETSERVER_H
#define TUNNELPROXYSOCKETSERVER_H

#include <QUuid>
#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcTunnelProxySocketServer)

#include "proxyconnection.h"
#include "tunnelproxysocket.h"

namespace remoteproxyclient {

class ProxyConnection;
class JsonRpcClient;

class TunnelProxySocketServer : public QObject
{
    Q_OBJECT
public:
    enum State {
        StateConnecting,
        StateHostLookup,
        StateConnected,
        StateInitializing,
        StateRegister,
        StateRunning,
        StateDiconnecting,
        StateDisconnected,
    };
    Q_ENUM(State)

    enum ConnectionType {
        ConnectionTypeWebSocket,
        ConnectionTypeTcpSocket
    };
    Q_ENUM(ConnectionType)

    explicit TunnelProxySocketServer(const QUuid &serverUuid, const QString &serverName, QObject *parent = nullptr);
    explicit TunnelProxySocketServer(const QUuid &serverUuid, const QString &serverName, ConnectionType connectionType, QObject *parent = nullptr);
    ~TunnelProxySocketServer();

    bool running() const;

    QAbstractSocket::SocketError error() const;

    void ignoreSslErrors();
    void ignoreSslErrors(const QList<QSslError> &errors);

public slots:
    void startServer(const QUrl &serverUrl);
    void stopServer();

signals:
    void runningChanged(bool running);

    void stateChanged(TunnelProxySocketServer::State state);
    void errorOccured(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &errors);


private slots:
    void onConnectionChanged(bool connected);
    void onConnectionDataAvailable(const QByteArray &data);

    void onConnectionSocketError(QAbstractSocket::SocketError error);
    void onConnectionStateChanged(QAbstractSocket::SocketState state);

    // Initialization calls
    void onHelloFinished();
    void onServerRegistrationFinished();

private:
    // This server information
    QUuid m_serverUuid;
    QString m_serverName;
    ConnectionType m_connectionType = ConnectionTypeTcpSocket;

    // Remote proxy server information
    QString m_remoteProxyServer;
    QString m_remoteProxyServerName;
    QString m_remoteProxyServerVersion;
    QString m_remoteProxyApiVersion;

    bool m_running = false;
    QUrl m_serverUrl;
    QAbstractSocket::SocketError m_error = QAbstractSocket::UnknownSocketError;
    State m_state = StateDisconnected;

    ProxyConnection *m_connection = nullptr;
    JsonRpcClient *m_jsonClient = nullptr;

    void setState(State state);
    void setRunning(bool running);
    void setError(QAbstractSocket::SocketError error);

    void cleanUp();
};

}

#endif // TUNNELPROXYSOCKETSERVER_H