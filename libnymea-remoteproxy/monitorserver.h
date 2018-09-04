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

#ifndef MONITORSERVER_H
#define MONITORSERVER_H

#include <QTimer>
#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

namespace remoteproxy {

class MonitorServer : public QObject
{
    Q_OBJECT
public:
    explicit MonitorServer(const QString &serverName, QObject *parent = nullptr);
    ~MonitorServer();

    bool running() const;

private:
    QString m_serverName;
    QLocalServer *m_server = nullptr;
    QList<QLocalSocket *> m_clients;

    void sendMonitorData(QLocalSocket *clientConnection, const QVariantMap &dataMap);

private slots:
    void onMonitorConnected();
    void onMonitorDisconnected();

public slots:
    void startServer();
    void stopServer();

    void updateClients(const QVariantMap &dataMap);
};

}

#endif // MONITORSERVER_H
