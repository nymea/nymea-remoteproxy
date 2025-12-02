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

#ifndef TUNNELPROXYHANDLER_H
#define TUNNELPROXYHANDLER_H

#include <QObject>

#include "jsonhandler.h"

namespace remoteproxy {

class TransportClient;

class TunnelProxyHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit TunnelProxyHandler(QObject *parent = nullptr);
    ~TunnelProxyHandler() override = default;

    QString name() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Server
    Q_INVOKABLE remoteproxy::JsonReply *RegisterServer(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE remoteproxy::JsonReply *DisconnectClient(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE remoteproxy::JsonReply *Ping(const QVariantMap &params, TransportClient *transportClient);

    // Client
    Q_INVOKABLE remoteproxy::JsonReply *RegisterClient(const QVariantMap &params, TransportClient *transportClient);
#else
    // Server
    Q_INVOKABLE JsonReply *RegisterServer(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE JsonReply *DisconnectClient(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE JsonReply *Ping(const QVariantMap &params, TransportClient *transportClient);

    // Client
    Q_INVOKABLE JsonReply *RegisterClient(const QVariantMap &params, TransportClient *transportClient);
#endif
signals:
    void ClientConnected(const QVariantMap &params, TransportClient *transportClient);
    void ClientDisconnected(const QVariantMap &params, TransportClient *transportClient);

};

}

#endif // TUNNELPROXYHANDLER_H
