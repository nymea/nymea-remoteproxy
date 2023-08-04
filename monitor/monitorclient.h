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
