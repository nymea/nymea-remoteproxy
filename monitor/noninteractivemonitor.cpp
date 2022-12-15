/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "noninteractivemonitor.h"
#include "utils.h"

#include <QDebug>

NonInteractiveMonitor::NonInteractiveMonitor(const QString &serverName, QObject *parent)
    : QObject{parent}
{
    m_monitorClient = new MonitorClient(serverName, this);
    connect(m_monitorClient, &MonitorClient::connected, this, &NonInteractiveMonitor::onConnected);

    m_monitorClient->connectMonitor();
}

void NonInteractiveMonitor::onConnected()
{
    connect(m_monitorClient, &MonitorClient::dataReady, this, [](const QVariantMap &dataMap){
        qInfo().noquote() << "---------------------------------------------------------------------";
        qInfo().noquote() << "Server name:" << dataMap.value("serverName", "-").toString();
        qInfo().noquote() << "Server version:" << dataMap.value("serverVersion", "-").toString();
        qInfo().noquote() << "API version:" << dataMap.value("apiVersion", "-").toString();
        qInfo().noquote() << "Total client count:" << dataMap.value("tunnelProxyStatistic").toMap().value("totalClientCount", 0).toInt();
        qInfo().noquote() << "Server connections:" << dataMap.value("tunnelProxyStatistic").toMap().value("serverConnectionsCount", 0).toInt();
        qInfo().noquote() << "Client connections:" << dataMap.value("tunnelProxyStatistic").toMap().value("clientConnectionsCount", 0).toInt();
        qInfo().noquote() << "Data troughput:" << Utils::humanReadableTraffic(dataMap.value("tunnelProxyStatistic").toMap().value("troughput", 0).toInt()) + " / s";
        qInfo().noquote() << "---------------------------------------------------------------------";

        QVariantMap tunnelProxyMap = dataMap.value("tunnelProxyStatistic").toMap();
        foreach (const QVariant &serverVariant, tunnelProxyMap.value("tunnelConnections").toList()) {
            QVariantMap serverMap = serverVariant.toMap();
            QVariantList clientList = serverMap.value("clientConnections").toList();

            // Server line
            QString serverConnectionTime = QDateTime::fromTime_t(serverMap.value("timestamp").toUInt()).toString("dd.MM.yyyy hh:mm:ss");
            QString serverLinePrint;
            if (clientList.isEmpty()) {
                serverLinePrint.prepend("├──");
            } else {
                serverLinePrint.prepend("├┬─");
            }

            serverLinePrint += QString("%1 | %2 RX: %3 TX: %4 | %5")
                    .arg(serverConnectionTime)
                    .arg(serverMap.value("address").toString(), - 15)
                    .arg(Utils::humanReadableTraffic(serverMap.value("rxDataCount").toInt()), - 9)
                    .arg(Utils::humanReadableTraffic(serverMap.value("txDataCount").toInt()), - 9)
                    .arg(serverMap.value("name").toString());

            qInfo().noquote() << serverLinePrint;

            for (int cc = 0; cc < clientList.count(); cc++) {
                QVariantMap clientMap = clientList.at(cc).toMap();
                QString clientLinePrint;
                if (cc >= clientList.count() - 1) {
                    clientLinePrint.append("│└─");
                } else {
                    clientLinePrint.prepend("│├─");
                }

                clientLinePrint += QString("%1 | %2 RX: %3 TX: %4 | %5")
                        .arg(QDateTime::fromTime_t(clientMap.value("timestamp").toUInt()).toString("dd.MM.yyyy hh:mm:ss"))
                        .arg(clientMap.value("address").toString(), - 15)
                        .arg(Utils::humanReadableTraffic(serverMap.value("rxDataCount").toInt()), - 9)
                        .arg(Utils::humanReadableTraffic(serverMap.value("txDataCount").toInt()), - 9)
                        .arg(clientMap.value("name").toString(), -30);

                qInfo().noquote() << clientLinePrint;
            }
        }

        exit(0);
    });

}
