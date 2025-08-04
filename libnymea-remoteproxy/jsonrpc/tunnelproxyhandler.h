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

#ifndef TUNNELPROXYHANDLER_H
#define TUNNELPROXYHANDLER_H

#include <QObject>

#include "jsonhandler.h"

namespace remoteproxy {

class TransportClient;

class TunnelProxyHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit TunnelProxyHandler(QObject *parent = nullptr);
    ~TunnelProxyHandler() override = default;

    QString name() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Server
    Q_INVOKABLE remoteproxy::JsonReply *RegisterServer(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE remoteproxy::JsonReply *DisconnectClient(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE remoteproxy::JsonReply *Ping(const QVariantMap &params, TransportClient *transportClient);

    // Client
    Q_INVOKABLE remoteproxy::JsonReply *RegisterClient(const QVariantMap &params, TransportClient *transportClient);
#else
    // Server
    Q_INVOKABLE JsonReply *RegisterServer(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE JsonReply *DisconnectClient(const QVariantMap &params, TransportClient *transportClient);
    Q_INVOKABLE JsonReply *Ping(const QVariantMap &params, TransportClient *transportClient);

    // Client
    Q_INVOKABLE JsonReply *RegisterClient(const QVariantMap &params, TransportClient *transportClient);
#endif
signals:
    void ClientConnected(const QVariantMap &params, TransportClient *transportClient);
    void ClientDisconnected(const QVariantMap &params, TransportClient *transportClient);

};

}

#endif // TUNNELPROXYHANDLER_H
