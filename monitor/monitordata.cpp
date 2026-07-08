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

#include "monitordata.h"

namespace {

TunnelClientInfo parseClient(const QVariantMap &clientMap)
{
    TunnelClientInfo client;
    client.id = QUuid(clientMap.value("id").toString());
    client.clientUuid = QUuid(clientMap.value("clientUuid").toString());
    client.address = QHostAddress(clientMap.value("address").toString());
    client.name = clientMap.value("name").toString();
    client.timestamp = clientMap.value("timestamp").toUInt();
    client.rxDataCount = clientMap.value("rxDataCount").toULongLong();
    client.txDataCount = clientMap.value("txDataCount").toULongLong();
    return client;
}

TunnelConnectionInfo parseTunnelConnection(const QVariantMap &serverMap)
{
    TunnelConnectionInfo connection;
    connection.id = QUuid(serverMap.value("id").toString());
    connection.serverUuid = QUuid(serverMap.value("serverUuid").toString());
    connection.address = QHostAddress(serverMap.value("address").toString());
    connection.name = serverMap.value("name").toString();
    connection.timestamp = serverMap.value("timestamp").toUInt();
    connection.rxDataCount = serverMap.value("rxDataCount").toULongLong();
    connection.txDataCount = serverMap.value("txDataCount").toULongLong();

    const QVariantList clientList = serverMap.value("clientConnections").toList();
    connection.clients.reserve(clientList.count());
    for (const QVariant &clientVariant : clientList)
        connection.clients.append(parseClient(clientVariant.toMap()));

    return connection;
}

TunnelProxyStatistics parseTunnelProxyStatistics(const QVariantMap &tunnelProxyMap)
{
    TunnelProxyStatistics statistics;
    statistics.totalClientCount = tunnelProxyMap.value("totalClientCount", 0).toInt();
    statistics.serverConnectionsCount = tunnelProxyMap.value("serverConnectionsCount", 0).toInt();
    statistics.clientConnectionsCount = tunnelProxyMap.value("clientConnectionsCount", 0).toInt();
    statistics.throughputBytesPerSecond = tunnelProxyMap.value("troughput", 0).toULongLong();

    const QVariantMap transportsMap = tunnelProxyMap.value("transports").toMap();
    for (auto it = transportsMap.constBegin(); it != transportsMap.constEnd(); ++it)
        statistics.transportConnectionCounts.insert(it.key(), it.value().toInt());

    const QVariantList tunnelConnectionsList = tunnelProxyMap.value("tunnelConnections").toList();
    statistics.tunnelConnections.reserve(tunnelConnectionsList.count());
    for (const QVariant &serverVariant : tunnelConnectionsList)
        statistics.tunnelConnections.append(parseTunnelConnection(serverVariant.toMap()));

    return statistics;
}

} // namespace

MonitorSnapshot MonitorData::fromVariant(const QVariantMap &data)
{
    MonitorSnapshot snapshot;
    snapshot.serverName = data.value("serverName", "-").toString();
    snapshot.serverVersion = data.value("serverVersion", "-").toString();
    snapshot.apiVersion = data.value("apiVersion", "-").toString();
    snapshot.tunnelProxyStatistic = parseTunnelProxyStatistics(data.value("tunnelProxyStatistic").toMap());
    return snapshot;
}
