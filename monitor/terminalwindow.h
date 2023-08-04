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
