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
#include "utils.h"

#include <QDebug>
#include <QTextStream>

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

        QVariantMap tunnelProxyMap = dataMap.value("tunnelProxyStatistic").toMap();

        qStdOut() << "---------------------------------------------------------------------\n";
        qStdOut() << "Server name:" << dataMap.value("serverName", "-").toString() << "\n";
        qStdOut() << "Server version:" << dataMap.value("serverVersion", "-").toString() << "\n";
        qStdOut() << "API version:" << dataMap.value("apiVersion", "-").toString() << "\n";
        qStdOut() << "Total client count:" << tunnelProxyMap.value("totalClientCount", 0).toInt() << "\n";
        qStdOut() << "Server connections:" << tunnelProxyMap.value("serverConnectionsCount", 0).toInt() << "\n";
        qStdOut() << "Client connections:" << tunnelProxyMap.value("clientConnectionsCount", 0).toInt() << "\n";
        qStdOut() << "Data troughput:" << Utils::humanReadableTraffic(tunnelProxyMap.value("troughput", 0).toInt()) + " / s" << "\n";
        qStdOut() << "---------------------------------------------------------------------" << "\n";
        QVariantMap transportsMap = tunnelProxyMap.value("transports").toMap();
        foreach(const QString &transportInterface, transportsMap.keys()) {
            qStdOut() << "Connections on " << transportInterface << ": " << transportsMap.value(transportInterface).toInt() << "\n";
        }
        qStdOut() << "---------------------------------------------------------------------" << "\n";

        foreach (const QVariant &serverVariant, tunnelProxyMap.value("tunnelConnections").toList()) {
            QVariantMap serverMap = serverVariant.toMap();
            QVariantList clientList = serverMap.value("clientConnections").toList();

            // Server line
            QString serverConnectionTime = QDateTime::fromMSecsSinceEpoch(serverMap.value("timestamp").toLongLong() * 1000).toString("dd.MM.yyyy hh:mm:ss");
            QString serverLinePrint;
            if (clientList.isEmpty()) {
                serverLinePrint.prepend("├──");
            } else {
                serverLinePrint.prepend("├┬─");
            }

            serverLinePrint += QString("%1 | %2 | %3 RX: %4 TX: %5 | %6")
                    .arg(serverConnectionTime)
                    .arg(serverMap.value("serverUuid").toString())
                    .arg(serverMap.value("address").toString(), - 15)
                    .arg(Utils::humanReadableTraffic(serverMap.value("rxDataCount").toInt()), - 9)
                    .arg(Utils::humanReadableTraffic(serverMap.value("txDataCount").toInt()), - 9)
                    .arg(serverMap.value("name").toString());

            qStdOut() << serverLinePrint << "\n";

            for (int cc = 0; cc < clientList.count(); cc++) {
                QVariantMap clientMap = clientList.at(cc).toMap();
                QString clientLinePrint;
                if (cc >= clientList.count() - 1) {
                    clientLinePrint.append("│└─");
                } else {
                    clientLinePrint.prepend("│├─");
                }

                clientLinePrint += QString("%1 | %2 | %3 RX: %4 TX: %5 | %6")
                        .arg(QDateTime::fromMSecsSinceEpoch(clientMap.value("timestamp").toLongLong() * 1000).toString("dd.MM.yyyy hh:mm:ss"))
                        .arg(clientMap.value("clientUuid").toString())
                        .arg(clientMap.value("address").toString(), - 15)
                        .arg(Utils::humanReadableTraffic(serverMap.value("rxDataCount").toInt()), - 9)
                        .arg(Utils::humanReadableTraffic(serverMap.value("txDataCount").toInt()), - 9)
                        .arg(clientMap.value("name").toString(), -30);

                qStdOut() << clientLinePrint << "\n";
            }
        }

        exit(0);
    });

    m_monitorClient->refresh();
}
