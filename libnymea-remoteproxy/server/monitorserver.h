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
    void onMonitorReadyRead();

    void processRequest(QLocalSocket *clientConnection, const QVariantMap &request);

public slots:
    void startServer();
    void stopServer();

    void updateClients(const QVariantMap &dataMap);
};

}

#endif // MONITORSERVER_H
