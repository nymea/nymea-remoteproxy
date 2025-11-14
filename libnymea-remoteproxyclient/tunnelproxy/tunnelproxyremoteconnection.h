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

#ifndef TUNNELPROXYREMOTECONNECTION_H
#define TUNNELPROXYREMOTECONNECTION_H

#include <QUrl>
#include <QUuid>
#include <QObject>
#include <QSslError>
#include <QAbstractSocket>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcTunnelProxyRemoteConnection)

namespace remoteproxyclient {

class JsonRpcClient;
class ProxyConnection;

class TunnelProxyRemoteConnection : public QObject
{
    Q_OBJECT
public:

    enum State {
        StateConnecting,
        StateHostLookup,
        StateConnected,
        StateInitializing,
        StateRegister,
        StateRemoteConnected,
        StateDiconnecting,
        StateDisconnected,
    };
    Q_ENUM(State)

    enum ConnectionType {
        ConnectionTypeWebSocket,
        ConnectionTypeTcpSocket
    };
    Q_ENUM(ConnectionType)

    explicit TunnelProxyRemoteConnection(const QUuid &clientUuid, const QString &clientName, QObject *parent = nullptr);
    explicit TunnelProxyRemoteConnection(const QUuid &clientUuid, const QString &clientName, ConnectionType connectionType, QObject *parent = nullptr);
    ~TunnelProxyRemoteConnection();

    bool remoteConnected() const;

    State state() const;

    QAbstractSocket::SocketError error() const;

    void ignoreSslErrors();
    void ignoreSslErrors(const QList<QSslError> &errors);

    QUrl serverUrl() const;

    QString remoteProxyServer() const;
    QString remoteProxyServerName() const;
    QString remoteProxyServerVersion() const;
    QString remoteProxyApiVersion() const;

public slots:
    bool connectServer(const QUrl &url, const QUuid &serverUuid);
    void disconnectServer();
    bool sendData(const QByteArray &data);

signals:
    void stateChanged(TunnelProxyRemoteConnection::State state);
    void errorOccurred(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &errors);

    void remoteConnectedChanged(bool remoteConnected);

    void dataReady(const QByteArray &data);

private slots:
    void onConnectionChanged(bool connected);
    void onConnectionDataAvailable(const QByteArray &data);

    void onConnectionSocketError(QAbstractSocket::SocketError error);
    void onConnectionStateChanged(QAbstractSocket::SocketState state);

    // Initialization calls
    void onHelloFinished();
    void onClientRegistrationFinished();

private:
    // This server information
    QUuid m_clientUuid;
    QString m_clientName;
    ConnectionType m_connectionType = ConnectionTypeTcpSocket;

    QUuid m_serverUuid;

    // Remote proxy server information
    QString m_remoteProxyServer;
    QString m_remoteProxyServerName;
    QString m_remoteProxyServerVersion;
    QString m_remoteProxyApiVersion;

    QUrl m_serverUrl;
    QAbstractSocket::SocketError m_error = QAbstractSocket::UnknownSocketError;
    State m_state = StateDisconnected;

    ProxyConnection *m_connection = nullptr;
    JsonRpcClient *m_jsonClient = nullptr;

    void setState(State state);
    void setRemoteConnected(bool remoteConnected);
    void setError(QAbstractSocket::SocketError error);

    void cleanUp();

};

}

#endif // TUNNELPROXYREMOTECONNECTION_H
