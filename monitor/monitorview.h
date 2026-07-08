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

#ifndef MONITORVIEW_H
#define MONITORVIEW_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>

#include <memory>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "monitordata.h"

// Fullscreen FTXUI view of the live tunnel-proxy connection tree.
//
// Driven by a QTimer calling ftxui::Loop::RunOnce() every tick, so
// QCoreApplication::exec() stays the one real event loop and QLocalSocket keeps
// running on the same thread as before - no worker thread/mutex bridge is needed
// for a view this simple.
class MonitorView : public QObject
{
    Q_OBJECT
public:
    explicit MonitorView(QObject *parent = nullptr);
    ~MonitorView() override;

public slots:
    void refreshView(const QVariantMap &dataMap);

signals:
    // Emitted once the user quits the view (Escape/q). Callers must delete this
    // object (not call exit() directly) so the terminal is restored first.
    void quitRequested();

private:
    ftxui::ScreenInteractive m_screen;
    std::unique_ptr<ftxui::Loop> m_loop;
    QTimer m_pollTimer;

    MonitorSnapshot m_snapshot;
    int m_scrollOffset = 0;

    ftxui::Element render();
    bool handleEvent(const ftxui::Event &event);

private slots:
    void poll();
};

#endif // MONITORVIEW_H
