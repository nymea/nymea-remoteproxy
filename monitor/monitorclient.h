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

#ifndef MONITORCLIENT_H
#define MONITORCLIENT_H

#include <QObject>
#include <QLocalSocket>

#include "terminalwindow.h"

class MonitorClient : public QObject
{
    Q_OBJECT
public:
    explicit MonitorClient(const QString &serverName, bool jsonMode, QObject *parent = nullptr);

    // Configuration before connection
    bool printAll() const;
    void setPrintAll(bool printAll);

private:
    QLocalSocket *m_socket = nullptr;

    QString m_serverName;
    bool m_jsonMode = false;
    bool m_printAll = false;
    QByteArray m_dataBuffer;

    void processBufferData();

signals:
    void connected();
    void disconnected();
    void dataReady(const QVariantMap &data);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QLocalSocket::LocalSocketError socketError);

public slots:
    void connectMonitor();
    void disconnectMonitor();

    void refresh();
};

#endif // MONITORCLIENT_H
