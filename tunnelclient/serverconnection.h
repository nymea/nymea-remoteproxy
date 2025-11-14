// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QUuid>
#include <QObject>

#include "tunnelproxy/tunnelproxysocketserver.h"

using namespace remoteproxyclient;

class ServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit ServerConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, bool insecure, bool echo, QObject *parent = nullptr);

    void startServer();

private:
    QUrl m_serverUrl;
    QString m_name;
    QUuid m_uuid;
    bool m_insecure = false;
    bool m_echo = false;

    TunnelProxySocketServer *m_socketServer = nullptr;
};

#endif // SERVERCONNECTION_H
