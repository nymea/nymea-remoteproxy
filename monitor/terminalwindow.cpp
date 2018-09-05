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

#include <QTime>
#include <QDebug>
#include <QDateTime>
#include <QMetaObject>

TerminalWindow::TerminalWindow(QObject *parent) :
    QObject(parent)
{
    // Create main window
    m_mainWindow = initscr();

    // Enable colors if available
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_GREEN);
        init_pair(2, COLOR_BLACK, COLOR_RED);
    }

    // Configure behaviour
    cbreak();
    noecho();
    nonl();;
    nodelay(stdscr, TRUE);
    curs_set(FALSE);

    clear();

    keypad(m_mainWindow, true);
    nodelay(m_mainWindow, true);
    getmaxyx(m_mainWindow, m_terminalSizeY, m_terminalSizeX);
    wrefresh(m_mainWindow);

    // Create header and content window
    m_headerWindow = newwin(m_headerHeight, m_terminalSizeX, 0, 0);
    m_contentWindow = newwin(m_terminalSizeY - m_headerHeight, m_terminalSizeX, m_headerHeight, 0);

    nodelay(m_headerWindow, TRUE);
    nodelay(m_contentWindow, TRUE);

    werase(m_headerWindow);
    werase(m_contentWindow);

    // Draw borders
    drawWindowBorder(m_headerWindow);
    drawWindowBorder(m_contentWindow);

    //box(m_headerWindow, 0 , 0);
    //box(m_contentWindow, 0 , 0);

    // Refresh windows
    wrefresh(m_headerWindow);
    wrefresh(m_contentWindow);

    refresh();

    eventLoop();
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

QString TerminalWindow::getDurationString(uint timestamp)
{
    uint duration = QDateTime::currentDateTime().toTime_t() - timestamp;
    QString res;
    int seconds = static_cast<int>(duration % 60);
    duration /= 60;
    int minutes = static_cast<int>(duration % 60);
    duration /= 60;
    int hours = static_cast<int>(duration % 24);
    return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
}

QString TerminalWindow::humanReadableTraffic(int bytes)
{
    double dataCount = bytes;
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("B");

    while(dataCount >= 1024.0 && i.hasNext()) {
        unit = i.next();
        dataCount /= 1024.0;
    }
    return QString().setNum(dataCount,'f',2) + " " + unit;
}

void TerminalWindow::resizeWindow()
{
    int terminalSizeX;
    int terminalSizeY;
    getmaxyx(stdscr, terminalSizeY, terminalSizeX);

    // Resize the window if size has changed
    if (m_terminalSizeX != terminalSizeX || m_terminalSizeY != terminalSizeY) {
        m_terminalSizeX = terminalSizeX;
        m_terminalSizeY = terminalSizeY;
        wresize(m_headerWindow, m_headerHeight, m_terminalSizeX);
        wresize(m_contentWindow, m_terminalSizeY - m_headerHeight, m_terminalSizeX);
    }
}

void TerminalWindow::drawWindowBorder(WINDOW *window)
{
    int x, y, i;
    getmaxyx(window, y, x);

    // corners
    mvwprintw(window, 0, 0, "+");
    mvwprintw(window, y - 1, 0, "+");
    mvwprintw(window, 0, x - 1, "+");
    mvwprintw(window, y - 1, x - 1, "+");

    // sides
    for (i = 1; i < (y - 1); i++) {
        mvwprintw(window, i, 0, "|");
        mvwprintw(window, i, x - 1, "|");
    }

    // top and bottom
    for (i = 1; i < (x - 1); i++) {
        mvwprintw(window, 0, i, "-");
        mvwprintw(window, y - 1, i, "-");
    }
}

void TerminalWindow::paintHeader()
{
    QString headerString = QString(" Server: %1 (%2) | API: %3 | Clients: %4, %5 | Tunnels: %6, %7 | %8 | %9 | %10")
            .arg(m_dataMap.value("serverName", "-").toString())
            .arg(m_dataMap.value("serverVersion", "-").toString())
            .arg(m_dataMap.value("apiVersion", "-").toString())
            .arg(m_dataMap.value("proxyStatistic").toMap().value("clientCount", 0).toInt())
            .arg(m_dataMap.value("proxyStatistic").toMap().value("total").toMap().value("totalClientCount").toInt())
            .arg(m_dataMap.value("proxyStatistic").toMap().value("tunnelCount", 0).toInt())
            .arg(m_dataMap.value("proxyStatistic").toMap().value("total").toMap().value("totalTunnelCount").toInt())
            .arg(humanReadableTraffic(m_dataMap.value("proxyStatistic").toMap().value("troughput", 0).toInt()) + " / s", - 13)
            .arg(humanReadableTraffic(m_dataMap.value("proxyStatistic").toMap().value("total").toMap().value("totalTraffic").toInt()), - 10)
            .arg((m_view == ViewClients ? "-- Clients --" : "-- Tunnels --"));

    int delta = m_terminalSizeX - headerString.count();

    // Fill string for background color
    for (int i = 0; i < delta; i++)
        headerString.append(" ");


    wattron(m_headerWindow, A_BOLD | COLOR_PAIR(1));
    mvwprintw(m_headerWindow, 1, 2, convertString(headerString));
    wattroff(m_headerWindow, A_BOLD | COLOR_PAIR(1));
}

void TerminalWindow::paintContentClients()
{
    if (m_clientHash.isEmpty())
        return;

    int i = 1;
    foreach (const QVariant &clientVariant, m_clientHash.values()) {
        QVariantMap clientMap = clientVariant.toMap();

        uint timeStamp = clientMap.value("timestamp").toUInt();
        QString clientConnectionTime = QDateTime::fromTime_t(timeStamp).toString("dd.MM.yyyy hh:mm:ss");

        int rxDataCountBytes = clientMap.value("rxDataCount").toInt();
        int txDataCountBytes = clientMap.value("txDataCount").toInt();

        QString clientPrint = QString("%1 | %2 | %3 | RX: %4 | TX: %5 | %6 | %7 | %8")
                .arg(clientConnectionTime)
                .arg(clientMap.value("duration").toString())
                .arg(clientMap.value("address").toString(), - 16)
                .arg(humanReadableTraffic(rxDataCountBytes), - 10)
                .arg(humanReadableTraffic(txDataCountBytes), - 10)
                .arg((clientMap.value("authenticated").toBool() ? "A" : "-"))
                .arg((clientMap.value("tunnelConnected").toBool() ? "T" : "-"))
                .arg(clientMap.value("name").toString(), -30);

        mvwprintw(m_contentWindow, i, 2, convertString(clientPrint.trimmed()));
        i++;
    }

}

void TerminalWindow::paintContentTunnels()
{
    int i = 1;

    foreach (const QVariant &tunnelVaiant, m_dataMap.value("proxyStatistic").toMap().value("tunnels").toList()) {
        QVariantMap tunnelMap = tunnelVaiant.toMap();
        QVariantMap clientOne = m_clientHash.value(tunnelMap.value("clientOne").toString());
        QVariantMap clientTwo = m_clientHash.value(tunnelMap.value("clientTwo").toString());

        // Tunnel time
        uint timeStamp = tunnelMap.value("timestamp").toUInt();
        QString tunnelConnectionTime = QDateTime::fromTime_t(timeStamp).toString("dd.MM.yyyy hh:mm:ss");


        QString tunnelPrint = QString("%1 | %2 | %3 | %4 | %5 (%6) <---> %7 (%8)")
                .arg(tunnelConnectionTime)
                .arg(getDurationString(timeStamp))
                .arg(humanReadableTraffic(clientOne.value("rxDataCount").toInt() + clientOne.value("txDataCount").toInt()), - 10)
                .arg(clientOne.value("userName").toString())
                .arg(clientOne.value("name").toString())
                .arg(clientOne.value("address").toString())
                .arg(clientTwo.value("name").toString())
                .arg(clientTwo.value("address").toString())
                ;

        mvwprintw(m_contentWindow, i, 2, convertString(tunnelPrint));
        i++;
    }
}

void TerminalWindow::eventLoop()
{

    werase(m_headerWindow);
    werase(m_contentWindow);

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
        qDebug() << "Closing window monitor. Have a nice day!";
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

    // Draw borders
    drawWindowBorder(m_headerWindow);
    drawWindowBorder(m_contentWindow);

    wrefresh(m_headerWindow);
    wrefresh(m_contentWindow);
    refresh();

    // Reinvoke the method for the next event loop run
    //QMetaObject::invokeMethod(this, "eventLoop", Qt::QueuedConnection);
    QTimer::singleShot(100, this, &TerminalWindow::eventLoop);
}

void TerminalWindow::refreshWindow(const QVariantMap &dataMap)
{
    m_dataMap = dataMap;

    QVariantMap statisticMap = m_dataMap.value("proxyStatistic").toMap();

    m_clientHash.clear();

    foreach (const QVariant &clientVariant, statisticMap.value("clients").toList()) {
        QVariantMap clientMap = clientVariant.toMap();
        clientMap.insert("duration", getDurationString(clientMap.value("timestamp").toUInt()));
        m_clientHash.insert(clientMap.value("id").toString(), clientMap);
    }
}

