/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "monitor.h"

Monitor::Monitor(const QString &serverName, QObject *parent) : QObject(parent)
{
    m_monitorClient = new MonitorClient(serverName, this);
    connect(m_monitorClient, &MonitorClient::connected, this, &Monitor::onConnected);
    connect(m_monitorClient, &MonitorClient::disconnected, this, &Monitor::onDisconnected);

    m_monitorClient->connectMonitor();
}

void Monitor::onConnected()
{
    m_terminal = new TerminalWindow(this);
    connect(m_monitorClient, &MonitorClient::dataReady, m_terminal, &TerminalWindow::refreshWindow);
}

void Monitor::onDisconnected()
{
    if (!m_terminal)
        return;

    delete m_terminal;
    m_terminal = nullptr;
    qDebug() << "Monitor disconnected.";
    exit(0);
}
