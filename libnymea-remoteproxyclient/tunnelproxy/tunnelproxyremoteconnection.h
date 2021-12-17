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
