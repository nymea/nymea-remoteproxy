/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
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
