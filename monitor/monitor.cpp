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

#include "monitor.h"
#include <QJsonDocument>

Monitor::Monitor(const QString &serverName, bool jsonMode, QObject *parent) :
    QObject(parent),
    m_jsonMode(jsonMode)
{
    m_monitorClient = new MonitorClient(serverName, jsonMode, this);
    connect(m_monitorClient, &MonitorClient::connected, this, &Monitor::onConnected);
    connect(m_monitorClient, &MonitorClient::disconnected, this, &Monitor::onDisconnected);

    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, m_monitorClient, &MonitorClient::refresh);

    m_monitorClient->connectMonitor();
}

void Monitor::onConnected()
{
    if (!m_jsonMode) {
        m_terminal = new TerminalWindow(this);
        connect(m_monitorClient, &MonitorClient::dataReady, m_terminal, &TerminalWindow::refreshWindow);
    }

    refresh();
    m_timer.start();
}

void Monitor::onDisconnected()
{
    m_timer.stop();

    if (!m_terminal)
        return;

    delete m_terminal;
    m_terminal = nullptr;
    qDebug() << "Monitor disconnected.";
    exit(0);
}
