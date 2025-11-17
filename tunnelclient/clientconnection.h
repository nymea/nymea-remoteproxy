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

#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QUrl>
#include <QObject>
#include <QTimer>

#include "tunnelproxy/tunnelproxyremoteconnection.h"

using namespace remoteproxyclient;

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, const QUuid &serverUuid, bool insecure, bool sendRandomData, QObject *parent = nullptr);

    void connectToServer();

private:
    QUrl m_serverUrl;
    QString m_name;
    QUuid m_uuid;
    QUuid m_serverUuid;
    bool m_insecure = false;
    bool m_sendRandomData = false;

    TunnelProxyRemoteConnection *m_remoteConnection = nullptr;
    QTimer m_timer;

    QString generateRandomString(uint length) const;
};

#endif // CLIENTCONNECTION_H
