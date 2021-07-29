/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2021, nymea GmbH
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

#ifndef TUNNELPROXYCLIENTCONNECTION_H
#define TUNNELPROXYCLIENTCONNECTION_H

#include <QUuid>
#include <QObject>

namespace remoteproxy {

class TransportClient;

class TunnelProxyClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit TunnelProxyClientConnection(TransportClient *transportClient, const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid, QObject *parent = nullptr);

    TransportClient *transportClient() const;

    QUuid clientUuid() const;
    QString clientName() const;
    QUuid serverUuid() const;

signals:

private:
    TransportClient *m_transportClient = nullptr;

    QUuid m_clientUuid;
    QString m_clientName;
    QUuid m_serverUuid;

};

}

#endif // TUNNELPROXYCLIENTCONNECTION_H
