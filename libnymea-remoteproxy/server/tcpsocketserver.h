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

#ifndef TCPSOCKETSERVER_H
#define TCPSOCKETSERVER_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QTcpServer>
#include <QSslConfiguration>

#include "transportinterface.h"

namespace remoteproxy {

class SslClient: public QSslSocket
{
    Q_OBJECT

public:
    explicit SslClient(QObject *parent = nullptr);

    void startWaitingForEncrypted();

private:
    QTimer m_timer;

};

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    explicit SslServer(bool sslEnabled, const QSslConfiguration &config, QObject *parent = nullptr);
    ~SslServer() override = default;

signals:
    void socketConnected(QSslSocket *socket);
    void socketDisconnected(QSslSocket *socket);
    void dataAvailable(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    bool m_sslEnabled = false;
    QSslConfiguration m_config;

    QVector<SslClient *> m_clients;

};


class TcpSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~TcpSocketServer() override;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void killClientConnection(const QUuid &clientId, const QString &killReason) override;

    uint connectionsCount() const override;

    bool running() const override;

public slots:
    bool startServer() override;
    bool stopServer() override;

private:
    bool m_sslEnabled;
    QSslConfiguration m_sslConfiguration;

    QHash<QUuid, QSslSocket *> m_clientList;

    SslServer *m_server = nullptr;

private slots:
    void onDataAvailable(QSslSocket *client, const QByteArray &data);
    void onSocketConnected(QSslSocket *client);
    void onSocketDisconnected(QSslSocket *client);


};

}

#endif // TCPSOCKETSERVER_H
