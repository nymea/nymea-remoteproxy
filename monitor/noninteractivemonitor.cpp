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

#include "noninteractivemonitor.h"
#include "monitordata.h"
#include "utils.h"

#include <QDebug>
#include <QDateTime>
#include <QTextStream>

namespace {

QString formatTimestamp(uint timestamp)
{
    return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timestamp) * 1000).toString("dd.MM.yyyy hh:mm:ss");
}

} // namespace

NonInteractiveMonitor::NonInteractiveMonitor(const QString &serverName, bool jsonMode, bool printAll, QObject *parent)
    : QObject{parent},
      m_jsonMode{jsonMode}
{
    m_monitorClient = new MonitorClient(serverName, m_jsonMode, this);
    m_monitorClient->setPrintAll(printAll);

    connect(m_monitorClient, &MonitorClient::connected, this, &NonInteractiveMonitor::onConnected);

    m_monitorClient->connectMonitor();
}

void NonInteractiveMonitor::onConnected()
{
    connect(m_monitorClient, &MonitorClient::dataReady, this, [](const QVariantMap &dataMap){
        const MonitorSnapshot snapshot = MonitorData::fromVariant(dataMap);
        const TunnelProxyStatistics &statistics = snapshot.tunnelProxyStatistic;

        qStdOut() << "---------------------------------------------------------------------\n";
        qStdOut() << "Server name:" << snapshot.serverName << "\n";
        qStdOut() << "Server version:" << snapshot.serverVersion << "\n";
        qStdOut() << "API version:" << snapshot.apiVersion << "\n";
        qStdOut() << "Total client count:" << statistics.totalClientCount << "\n";
        qStdOut() << "Server connections:" << statistics.serverConnectionsCount << "\n";
        qStdOut() << "Client connections:" << statistics.clientConnectionsCount << "\n";
        qStdOut() << "Data troughput:" << Utils::humanReadableTraffic(statistics.throughputBytesPerSecond) + " / s" << "\n";
        qStdOut() << "---------------------------------------------------------------------" << "\n";
        for (auto it = statistics.transportConnectionCounts.constBegin(); it != statistics.transportConnectionCounts.constEnd(); ++it)
            qStdOut() << "Connections on " << it.key() << ": " << it.value() << "\n";
        qStdOut() << "---------------------------------------------------------------------" << "\n";

        for (const TunnelConnectionInfo &connection : statistics.tunnelConnections) {
            QString serverLinePrint = connection.clients.isEmpty() ? QStringLiteral("├──") : QStringLiteral("├┬─");
            serverLinePrint += QString("%1 | %2 | %3 RX: %4 TX: %5 | %6")
                    .arg(formatTimestamp(connection.timestamp))
                    .arg(connection.serverUuid.toString())
                    .arg(connection.address.toString(), -15)
                    .arg(Utils::humanReadableTraffic(connection.rxDataCount), -9)
                    .arg(Utils::humanReadableTraffic(connection.txDataCount), -9)
                    .arg(connection.name);

            qStdOut() << serverLinePrint << "\n";

            for (int cc = 0; cc < connection.clients.count(); cc++) {
                const TunnelClientInfo &client = connection.clients.at(cc);
                QString clientLinePrint = (cc >= connection.clients.count() - 1) ? QStringLiteral("│└─") : QStringLiteral("│├─");
                clientLinePrint += QString("%1 | %2 | %3 RX: %4 TX: %5 | %6")
                        .arg(formatTimestamp(client.timestamp))
                        .arg(client.clientUuid.toString())
                        .arg(client.address.toString(), -15)
                        .arg(Utils::humanReadableTraffic(client.rxDataCount), -9)
                        .arg(Utils::humanReadableTraffic(client.txDataCount), -9)
                        .arg(client.name, -30);

                qStdOut() << clientLinePrint << "\n";
            }
        }

        exit(0);
    });

    m_monitorClient->refresh();
}
