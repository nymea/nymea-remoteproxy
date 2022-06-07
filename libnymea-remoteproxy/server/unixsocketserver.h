/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
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

#ifndef UNIXSOCKETSERVER_H
#define UNIXSOCKETSERVER_H

#include <QObject>
#include <QUuid>
#include <QLocalServer>
#include <QLocalSocket>

#include "transportinterface.h"

namespace remoteproxy {

class UnixSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit UnixSocketServer(QString socketFileName, QObject *parent = nullptr);
    ~UnixSocketServer() override;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void killClientConnection(const QUuid &clientId, const QString &killReason) override;

    bool running() const override;

public slots:
    bool startServer() override;
    bool stopServer() override;

private:
    QString m_socketFileName;
    QLocalServer *m_server = nullptr;
    QHash<QUuid, QLocalSocket *> m_clientList;

private slots:
    void onClientConnected();

};

}

#endif // UNIXSOCKETSERVER_H
