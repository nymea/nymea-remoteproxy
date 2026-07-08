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

#ifndef MONITORDATA_H
#define MONITORDATA_H

#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QString>
#include <QUuid>
#include <QVariantMap>

// One client tunneled through a TunnelConnectionInfo's server.
struct TunnelClientInfo {
    QUuid id;
    QUuid clientUuid;
    QHostAddress address;
    QString name;
    uint timestamp = 0;
    quint64 rxDataCount = 0;
    quint64 txDataCount = 0;
};

// One server side of a tunnel, with the clients currently tunneled through it.
struct TunnelConnectionInfo {
    QUuid id;
    QUuid serverUuid;
    QHostAddress address;
    QString name;
    uint timestamp = 0;
    quint64 rxDataCount = 0;
    quint64 txDataCount = 0;
    QList<TunnelClientInfo> clients;
};

struct TunnelProxyStatistics {
    int totalClientCount = 0;
    int serverConnectionsCount = 0;
    int clientConnectionsCount = 0;
    quint64 throughputBytesPerSecond = 0;
    QMap<QString, int> transportConnectionCounts;
    QList<TunnelConnectionInfo> tunnelConnections;
};

struct MonitorSnapshot {
    QString serverName;
    QString serverVersion;
    QString apiVersion;
    TunnelProxyStatistics tunnelProxyStatistic;
};

namespace MonitorData {

// Single point that parses the monitor server's JSON schema (built by
// Engine::buildMonitorData()/TunnelProxyServer::currentStatistics()) into typed
// data. Keeping this in one place is what prevents field-name mixups (e.g. reading
// a server's rxDataCount for one of its clients) from being reintroduced.
MonitorSnapshot fromVariant(const QVariantMap &data);

}

#endif // MONITORDATA_H
