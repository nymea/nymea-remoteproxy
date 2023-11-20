/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2023, nymea GmbH
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

#include "terminalwindow.h"
#include "utils.h"

#include <QTime>
#include <QDebug>
#include <QDateTime>
#include <QMetaObject>

TerminalWindow::TerminalWindow(QObject *parent) :
    QObject(parent)
{
    // Init view tabs
    m_tabs << ViewTunnelProxy;

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
    //drawWindowBorder(m_contentWindow);

    scrollok(m_contentWindow, true);

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
    cleanup();
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

void TerminalWindow::moveTabRight()
{
    int currentIndex = m_tabs.indexOf(m_view);
    if (currentIndex + 1 >= m_tabs.count()) {
        m_view = m_tabs.at(0);
    } else {
        m_view = m_tabs.at( currentIndex + 1);
    }
}

void TerminalWindow::moveTabLeft()
{
    int currentIndex = m_tabs.indexOf(m_view);
    if (currentIndex - 1 < 0) {
        m_view = m_tabs.at(m_tabs.count() - 1);
    } else {
        m_view = m_tabs.at( currentIndex - 1);
    }
}

void TerminalWindow::paintHeader()
{
    QString windowName;
    QString headerString;
    switch (m_view) {
    case ViewTunnelProxy:
        windowName = "-- TunnelProxy --";
        headerString = QString(" Server: %1 (%2) | API: %3 | Total: %4 | Servers: %5 | Clients: %6 | %7 | %8")
                .arg(m_dataMap.value("serverName", "-").toString())
                .arg(m_dataMap.value("serverVersion", "-").toString())
                .arg(m_dataMap.value("apiVersion", "-").toString())
                .arg(m_dataMap.value("tunnelProxyStatistic").toMap().value("totalClientCount", 0).toInt())
                .arg(m_dataMap.value("tunnelProxyStatistic").toMap().value("serverConnectionsCount", 0).toInt())
                .arg(m_dataMap.value("tunnelProxyStatistic").toMap().value("clientConnectionsCount", 0).toInt())
                .arg(Utils::humanReadableTraffic(m_dataMap.value("tunnelProxyStatistic").toMap().value("troughput", 0).toInt()) + " / s", - 13)
                .arg(windowName);
        break;
    }

    int delta = m_terminalSizeX - headerString.count();

    // Fill string for background color
    for (int i = 0; i < delta; i++)
        headerString.append(" ");

    //wattron(m_headerWindow, A_BOLD | COLOR_PAIR(1));
    mvwprintw(m_headerWindow, 1, 2, "%s", headerString.toLatin1().constData());
    //wattroff(m_headerWindow, A_BOLD | COLOR_PAIR(1));
}

void TerminalWindow::paintContentClients()
{
    if (m_clientHash.isEmpty())
        return;

    int i = 1;
    foreach (const QVariant &clientVariant, m_clientHash.values()) {
        QVariantMap clientMap = clientVariant.toMap();

        uint timeStamp = clientMap.value("timestamp").toUInt();
        QString clientConnectionTime = QDateTime::fromMSecsSinceEpoch(timeStamp * 1000).toString("dd.MM.yyyy hh:mm:ss");

        int rxDataCountBytes = clientMap.value("rxDataCount").toInt();
        int txDataCountBytes = clientMap.value("txDataCount").toInt();

        QString clientPrint = QString("%1 | %2 | %3 | RX: %4 | TX: %5 | %6 | %7 | %8")
                .arg(clientConnectionTime)
                .arg(clientMap.value("duration").toString())
                .arg(clientMap.value("address").toString(), - 16)
                .arg(Utils::humanReadableTraffic(rxDataCountBytes), - 10)
                .arg(Utils::humanReadableTraffic(txDataCountBytes), - 10)
                .arg((clientMap.value("authenticated").toBool() ? "A" : "-"))
                .arg((clientMap.value("tunnelConnected").toBool() ? "T" : "-"))
                .arg(clientMap.value("name").toString(), -30);

        mvwprintw(m_contentWindow, i, 2, "%s", clientPrint.trimmed().toLatin1().constData());
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
        QString tunnelConnectionTime = QDateTime::fromMSecsSinceEpoch(timeStamp * 1000).toString("dd.MM.yyyy hh:mm:ss");

        QString tunnelPrint = QString("%1 | %2 | %3 | %4 | %5 (%6) <---> %7 (%8)")
                .arg(tunnelConnectionTime)
                .arg(Utils::getDurationString(timeStamp))
                .arg(Utils::humanReadableTraffic(clientOne.value("rxDataCount").toInt() + clientOne.value("txDataCount").toInt()), - 10)
                .arg(clientOne.value("userName").toString())
                .arg(clientOne.value("name").toString())
                .arg(clientOne.value("address").toString())
                .arg(clientTwo.value("name").toString())
                .arg(clientTwo.value("address").toString())
                ;

        mvwprintw(m_contentWindow, i, 2, "%s", tunnelPrint.toLatin1().constData());
        i++;
    }
}

void TerminalWindow::paintContentTunnelProxy()
{
    QVariantMap tunnelProxyMap = m_dataMap.value("tunnelProxyStatistic").toMap();
    int i = 1;

    foreach (const QVariant &serverVariant, tunnelProxyMap.value("tunnelConnections").toList()) {
        QVariantMap serverMap = serverVariant.toMap();
        uint timeStamp = serverMap.value("timestamp").toUInt();
        QString serverConnectionTime = QDateTime::fromMSecsSinceEpoch(timeStamp * 1000).toString("dd.MM.yyyy hh:mm:ss");
        int rxDataCountBytes = serverMap.value("rxDataCount").toInt();
        int txDataCountBytes = serverMap.value("txDataCount").toInt();
        QString serverLinePrint = QString("%1 | %2 | RX: %3 | TX: %4 | %5")
                .arg(serverConnectionTime)
                .arg(serverMap.value("address").toString(), - 16)
                .arg(Utils::humanReadableTraffic(rxDataCountBytes), - 10)
                .arg(Utils::humanReadableTraffic(txDataCountBytes), - 10)
                .arg(serverMap.value("name").toString(), -30);

        QVariantList clientList = serverMap.value("clientConnections").toList();
        mvwaddch(m_contentWindow, i, 2, ACS_LTEE);
        mvwaddch(m_contentWindow, i, 3, ACS_HLINE);
        if (clientList.isEmpty()) {
            mvwaddch(m_contentWindow, i, 4, ACS_HLINE);
        } else {
            mvwaddch(m_contentWindow, i, 4, ACS_TTEE);
        }
        mvwaddch(m_contentWindow, i, 5, ACS_HLINE);
        mvwprintw(m_contentWindow, i, 6, "%s", serverLinePrint.trimmed().toLatin1().constData());
        i++;

        for (int cc = 0; cc < clientList.count(); cc++) {
            QVariantMap clientMap = clientList.at(cc).toMap();

            mvwaddch(m_contentWindow, i, 2, ACS_VLINE);

            if (cc >= clientList.count() - 1) {
                mvwaddch(m_contentWindow, i, 4, ACS_LLCORNER);
            } else {
                mvwaddch(m_contentWindow, i, 4, ACS_LTEE);
            }
            mvwaddch(m_contentWindow, i, 5, ACS_HLINE);

            QString clientLinePrint = QString("%1 | %2 | RX: %3 | TX: %4 | %5")
                    .arg(QDateTime::fromMSecsSinceEpoch(clientMap.value("timestamp").toULongLong() * 1000).toString("dd.MM.yyyy hh:mm:ss"))
                    .arg(clientMap.value("address").toString(), - 16)
                    .arg(Utils::humanReadableTraffic(clientMap.value("rxDataCount").toInt()), - 10)
                    .arg(Utils::humanReadableTraffic(clientMap.value("txDataCount").toInt()), - 10)
                    .arg(clientMap.value("name").toString(), -30);

            mvwprintw(m_contentWindow, i, 6, "%s", clientLinePrint.trimmed().toLatin1().constData());

            i++;
        }
    }
}

void TerminalWindow::cleanup()
{
    clear();
    delwin(m_headerWindow);
    delwin(m_contentWindow);
    delwin(m_mainWindow);
    endwin();
}

void TerminalWindow::eventLoop()
{
    werase(m_headerWindow);
    werase(m_contentWindow);

    resizeWindow();

    int keyInteger = getch();
    switch (keyInteger) {
    case KEY_LEFT:
        moveTabLeft();
        break;
    case KEY_RIGHT:
        moveTabRight();
        break;
    case 27: // Esc
        cleanup();
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
    case ViewTunnelProxy:
        paintContentTunnelProxy();
        break;
    }

    switch (keyInteger) {
    case KEY_DOWN:
        scroll(m_contentWindow);
        scrl(1);
        break;
    case KEY_UP:
        scroll(m_contentWindow);
        scrl(-1);
        break;
    }

    // Draw borders
    drawWindowBorder(m_headerWindow);
    //drawWindowBorder(m_contentWindow);

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
        clientMap.insert("duration", Utils::getDurationString(clientMap.value("timestamp").toUInt()));
        m_clientHash.insert(clientMap.value("id").toString(), clientMap);
    }
}

