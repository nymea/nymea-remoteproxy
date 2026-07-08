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

#include "monitorview.h"
#include "utils.h"

#include <algorithm>
#include <string>

using namespace ftxui;

namespace {

// One line per server, followed by one indented line per client tunneled
// through it. Idle servers (no clients) are dimmed, active ones highlighted -
// the old ncurses code set up color pairs for exactly this but never used them.
void appendTunnelConnectionRows(Elements &rows, const TunnelConnectionInfo &connection)
{
    const Color rowColor = connection.clients.isEmpty() ? Color::GrayDark : Color::Green;
    rows.push_back(hbox({
        text(connection.clients.isEmpty() ? "─ " : "┬ ") | color(rowColor),
        text(connection.name.toStdString()) | bold | flex,
        text(connection.address.toString().toStdString()) | size(ftxui::WIDTH, ftxui::EQUAL, 16),
        text("RX " + Utils::humanReadableTraffic(static_cast<qint64>(connection.rxDataCount)).toStdString()) | size(ftxui::WIDTH, ftxui::EQUAL, 12),
        text("TX " + Utils::humanReadableTraffic(static_cast<qint64>(connection.txDataCount)).toStdString()) | size(ftxui::WIDTH, ftxui::EQUAL, 12),
    }));

    for (int i = 0; i < connection.clients.count(); ++i) {
        const TunnelClientInfo &client = connection.clients.at(i);
        const bool isLast = (i == connection.clients.count() - 1);
        rows.push_back(hbox({
            text(isLast ? "  └─ " : "  ├─ ") | color(Color::GrayDark),
            text(client.name.toStdString()) | flex,
            text(client.address.toString().toStdString()) | size(ftxui::WIDTH, ftxui::EQUAL, 16),
            text("RX " + Utils::humanReadableTraffic(static_cast<qint64>(client.rxDataCount)).toStdString()) | size(ftxui::WIDTH, ftxui::EQUAL, 12),
            text("TX " + Utils::humanReadableTraffic(static_cast<qint64>(client.txDataCount)).toStdString()) | size(ftxui::WIDTH, ftxui::EQUAL, 12),
        }));
    }
}

} // namespace

MonitorView::MonitorView(QObject *parent) :
    QObject(parent),
    m_screen(ScreenInteractive::Fullscreen())
{
    Component component = Renderer([this] { return render(); });
    component |= CatchEvent([this](const Event &event) { return handleEvent(event); });

    m_loop = std::make_unique<Loop>(&m_screen, component);

    connect(&m_pollTimer, &QTimer::timeout, this, &MonitorView::poll);
    m_pollTimer.start(50);
}

MonitorView::~MonitorView()
{
    // Explicitly destroy before the base QObject teardown continues: Loop's
    // destructor is what restores the terminal (raw mode, alternate screen).
    m_loop.reset();
}

void MonitorView::poll()
{
    m_loop->RunOnce();

    if (m_loop->HasQuitted()) {
        m_pollTimer.stop();
        emit quitRequested();
    }
}

void MonitorView::refreshView(const QVariantMap &dataMap)
{
    m_snapshot = MonitorData::fromVariant(dataMap);
}

bool MonitorView::handleEvent(const Event &event)
{
    if (event == Event::Escape || event == Event::Character('q') || event == Event::Character('Q')) {
        m_screen.Exit();
        return true;
    }

    if (event == Event::ArrowUp) {
        m_scrollOffset = std::max(0, m_scrollOffset - 1);
        return true;
    }

    if (event == Event::ArrowDown) {
        m_scrollOffset++;
        return true;
    }

    return false;
}

Element MonitorView::render()
{
    const TunnelProxyStatistics &statistics = m_snapshot.tunnelProxyStatistic;

    Element header = hbox({
                          text(" " + m_snapshot.serverName.toStdString() + " ") | bold,
                          text("(" + m_snapshot.serverVersion.toStdString() + ")"),
                          text(" | API " + m_snapshot.apiVersion.toStdString()),
                          text(" | Servers " + std::to_string(statistics.serverConnectionsCount)),
                          text(" | Clients " + std::to_string(statistics.clientConnectionsCount)),
                          text(" | " + Utils::humanReadableTraffic(static_cast<qint64>(statistics.throughputBytesPerSecond)).toStdString() + "/s"),
                          filler(),
                          text(" nymea-remoteproxy-monitor "),
                      })
            | bgcolor(Color::Blue) | color(Color::White);

    Elements rows;
    for (const TunnelConnectionInfo &connection : statistics.tunnelConnections)
        appendTunnelConnectionRows(rows, connection);

    if (rows.empty())
        rows.push_back(text("No tunnel connections.") | dim);

    const int visibleRows = std::max(1, m_screen.dimy() - 3);
    const int totalRows = static_cast<int>(rows.size());
    m_scrollOffset = std::min(m_scrollOffset, std::max(0, totalRows - visibleRows));

    Elements visible(rows.begin() + m_scrollOffset,
                      rows.begin() + std::min(totalRows, m_scrollOffset + visibleRows));

    return vbox({
        header,
        separator(),
        vbox(std::move(visible)) | flex,
    });
}
