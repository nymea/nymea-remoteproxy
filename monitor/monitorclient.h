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

#ifndef MONITORCLIENT_H
#define MONITORCLIENT_H

#include <QObject>
#include <QLocalSocket>

#include "terminalwindow.h"

class MonitorClient : public QObject
{
    Q_OBJECT
public:
    explicit MonitorClient(const QString &serverName, QObject *parent = nullptr);

private:
    QString m_serverName;
    QLocalSocket *m_socket = nullptr;
signals:
    void dataReady(const QVariantMap &data);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();

public slots:
    void connectMonitor();
    void disconnectMonitor();

};

#endif // MONITORCLIENT_H
