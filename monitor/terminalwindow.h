#ifndef TERMINALWINDOW_H
#define TERMINALWINDOW_H

#include <QObject>
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

#include <QTimer>
#include <QVariantMap>

#include <ncurses.h>

class TerminalWindow : public QObject
{
    Q_OBJECT
public:

    enum View {
        ViewClients,
        ViewTunnels
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

    View m_view = ViewClients;

    QVariantMap m_dataMap;
    QHash<QString, QVariantMap> m_clientHash;

    const char *convertString(const QString &string);
    QString getDurationString(uint timestamp);

    QString humanReadableTraffic(int bytes);

    // content paint methods
    void resizeWindow();
    void drawWindowBorder(WINDOW *window);
    void paintHeader();
    void paintContentClients();
    void paintContentTunnels();

signals:

private slots:
    void eventLoop();

public slots:
    void refreshWindow(const QVariantMap &dataMap);

};

#endif // TERMINALWINDOW_H
