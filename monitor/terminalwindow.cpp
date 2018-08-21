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

#include "terminalwindow.h"

#include <QDebug>
#include <QDateTime>
#include <QMetaObject>

TerminalWindow::TerminalWindow(QObject *parent) :
    QObject(parent)
{
    // Create main window
    m_mainWindow = initscr();
    start_color();

    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_BLACK, COLOR_RED);

    // Configure behaviour
    cbreak();
    noecho();
    curs_set(FALSE);

    //clear();

    keypad(m_mainWindow, true);
    nodelay(m_mainWindow, true);
    getmaxyx(stdscr, m_terminalSizeY, m_terminalSizeX);

    // Create header and content window
    m_headerWindow = newwin(m_headerHeight, m_terminalSizeX, 0, 0);
    m_contentWindow = newwin(m_terminalSizeY - m_headerHeight, m_terminalSizeX, m_headerHeight, 0);

    // Draw borders
    wclear(m_headerWindow);
    wclear(m_contentWindow);

    mvwprintw(m_headerWindow, 1, 2, convertString("Connecting..."));

    box(m_headerWindow, 0 , 0);
    box(m_contentWindow, 0 , 0);

    // Refresh windows
    wrefresh(m_headerWindow);
    wrefresh(m_contentWindow);

    refresh();
}

TerminalWindow::~TerminalWindow()
{
    clear();
    delwin(m_headerWindow);
    delwin(m_contentWindow);
    delwin(m_mainWindow);
    endwin();
}

const char *TerminalWindow::convertString(const QString &string)
{
    return reinterpret_cast<const char *>(string.toLatin1().data());
}

void TerminalWindow::resizeWindow()
{
    int terminalSizeX;
    int terminalSizeY;
    getmaxyx(stdscr, terminalSizeY, terminalSizeX);
    if (m_terminalSizeX != terminalSizeX || m_terminalSizeY != terminalSizeY) {
        m_terminalSizeX = terminalSizeX;
        m_terminalSizeY = terminalSizeY;
        wresize(m_headerWindow, m_headerHeight, m_terminalSizeX);
        wresize(m_contentWindow, m_terminalSizeY - m_headerHeight, m_terminalSizeX);
    }
}

void TerminalWindow::paintHeader()
{
    QString headerString = QString(" Server: %1 %2 | API version %3 | Clients: %4 | Tunnels %5 | %6 %7")
            .arg(m_dataMap.value("serverName").toString())
            .arg(m_dataMap.value("serverVersion").toString())
            .arg(m_dataMap.value("apiVersion").toString())
            .arg(m_dataMap.value("proxyStatistic").toMap().value("clientCount").toInt())
            .arg(m_dataMap.value("proxyStatistic").toMap().value("tunnelCount").toInt())
            .arg((m_view == ViewClients ? "Clients" : "Tunnels"))
            .arg("", m_terminalSizeX, ' ');


    wclear(m_headerWindow);
    wattron(m_headerWindow, A_BOLD | COLOR_PAIR(1));
    mvwprintw(m_headerWindow, 1, 2, convertString(headerString));
    wattroff(m_headerWindow, A_BOLD | COLOR_PAIR(1));
    box(m_headerWindow, 0 , 0);
    wrefresh(m_headerWindow);
}

void TerminalWindow::paintContentClients()
{
    int i = 1;

    wclear(m_contentWindow);
    foreach (const QVariant &clientVariant, m_clientHash.values()) {
        QVariantMap clientMap = clientVariant.toMap();

        uint timeStamp = clientMap.value("timestamp").toUInt();
        QString clientConnectionTime = QDateTime::fromTime_t(timeStamp).toString("dd.MM.yyyy hh:mm:ss");

        QString clientPrint = QString("%1 | %2 | %3 | %4 | %5 | %6 |")
                .arg(clientConnectionTime)
                .arg(clientMap.value("address").toString())
                .arg(clientMap.value("uuid").toString())
                .arg(clientMap.value("name").toString(), -30)
                .arg((clientMap.value("authenticated").toBool() ? "auth" : "no auth"), -9)
                .arg((clientMap.value("tunnelConnected").toBool() ? "<--->" : "  |  "));

        mvwprintw(m_contentWindow, i, 2, convertString(clientPrint));
        i++;
    }

    // Draw borders
    box(m_contentWindow, 0 , 0);
    wrefresh(m_contentWindow);
}

void TerminalWindow::paintContentTunnels()
{
    int i = 1;

    wclear(m_contentWindow);
    foreach (const QVariant &tunnelVaiant, m_dataMap.value("proxyStatistic").toMap().value("tunnels").toList()) {
        QVariantMap tunnelMap = tunnelVaiant.toMap();
        QVariantMap clientOne = m_clientHash.value(tunnelMap.value("clientOne").toString());
        QVariantMap clientTwo = m_clientHash.value(tunnelMap.value("clientTwo").toString());

        // Tunnel time
        uint timeStamp = tunnelMap.value("timestamp").toUInt();
        QString tunnelConnectionTime = QDateTime::fromTime_t(timeStamp).toString("dd.MM.yyyy hh:mm:ss");

        QString tunnelPrint = QString("%1 | %2 %3 %4 %5 %6")
                .arg(tunnelConnectionTime)
                .arg(clientOne.value("address").toString())
                .arg(clientOne.value("name").toString())
                .arg("<--->", 10)
                .arg(clientTwo.value("address").toString())
                .arg(clientTwo.value("name").toString())
                ;

        mvwprintw(m_contentWindow, i, 2, convertString(tunnelPrint));
        i++;
    }

    // Draw borders
    box(m_contentWindow, 0 , 0);
    wrefresh(m_contentWindow);
}

void TerminalWindow::eventLoop()
{
    resizeWindow();

    int keyInteger = getch();
    switch (keyInteger) {
    case KEY_LEFT:
        m_view = ViewClients;
        break;
    case KEY_RIGHT:
        m_view = ViewTunnels;
        break;
    case 27: // Esc
        clear();
        delwin(m_headerWindow);
        delwin(m_contentWindow);
        delwin(m_mainWindow);
        endwin();
        exit(0);
        break;
    default:
        break;
    }

    // Refresh header window
    paintHeader();

    // Refresh content window
    switch (m_view) {
    case ViewClients:
        paintContentClients();
        break;
    case ViewTunnels:
        paintContentTunnels();
        break;
    }

    refresh();

    // Reinvoke the method for the next event loop run
    QMetaObject::invokeMethod(this, "eventLoop", Qt::QueuedConnection);
}


void TerminalWindow::refreshWindow(const QVariantMap &dataMap)
{
    m_dataMap = dataMap;

    QVariantMap statisticMap = m_dataMap.value("proxyStatistic").toMap();

    m_clientHash.clear();
    foreach (const QVariant &clientVariant, statisticMap.value("clients").toList()) {
        QVariantMap clientMap = clientVariant.toMap();
        m_clientHash.insert(clientMap.value("id").toString(), clientMap);
    }

    eventLoop();
}

