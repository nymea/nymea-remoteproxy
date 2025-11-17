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

#ifndef UNIXSOCKETSERVER_H
#define UNIXSOCKETSERVER_H

#include <QObject>
#include <QUuid>
#include <QLocalServer>
#include <QLocalSocket>

#include "transportinterface.h"

namespace remoteproxy {

class UnixSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit UnixSocketServer(QString socketFileName, QObject *parent = nullptr);
    ~UnixSocketServer() override;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void killClientConnection(const QUuid &clientId, const QString &killReason) override;

    uint connectionsCount() const override;

    bool running() const override;

public slots:
    bool startServer() override;
    bool stopServer() override;

private:
    QString m_socketFileName;
    QLocalServer *m_server = nullptr;
    QHash<QUuid, QLocalSocket *> m_clientList;

private slots:
    void onClientConnected();

};

}

#endif // UNIXSOCKETSERVER_H
