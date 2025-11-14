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

#ifndef TERMINALWINDOW_H
#define TERMINALWINDOW_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>

#include <ncurses.h>

class TerminalWindow : public QObject
{
    Q_OBJECT
public:

    enum View {
        ViewTunnelProxy
    };
    Q_ENUM(View)

    explicit TerminalWindow(QObject *parent = nullptr);
    ~TerminalWindow();

private:
    WINDOW *m_mainWindow = nullptr;
    WINDOW *m_headerWindow = nullptr;
    WINDOW *m_contentWindow = nullptr;

    int m_headerHeight = 3;
    int m_terminalSizeX = 0;
    int m_terminalSizeY = 0;

    View m_view = ViewTunnelProxy;
    int m_tunnelProxyScollIndex = 0;


    // Tabs
    QList<View> m_tabs;

    QVariantMap m_dataMap;
    QHash<QString, QVariantMap> m_clientHash;


    // content paint methods
    void resizeWindow();
    void drawWindowBorder(WINDOW *window);
    void moveTabRight();
    void moveTabLeft();

    void paintHeader();
    void paintContentClients();
    void paintContentTunnels();
    void paintContentTunnelProxy();

    void cleanup();

private slots:
    void eventLoop();

public slots:
    void refreshWindow(const QVariantMap &dataMap);

};

#endif // TERMINALWINDOW_H
