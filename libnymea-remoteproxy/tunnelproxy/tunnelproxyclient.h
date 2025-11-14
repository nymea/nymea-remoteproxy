// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TUNNELPROXYCLIENT_H
#define TUNNELPROXYCLIENT_H

#include <QObject>
#include <QTimer>

#include "server/transportclient.h"

namespace remoteproxy {

class TunnelProxyClient : public TransportClient
{
    Q_OBJECT
public:
    enum Type {
        TypeNone,
        TypeServer,
        TypeClient
    };
    Q_ENUM(Type)

    explicit TunnelProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent = nullptr);

    Type type() const;
    void setType(Type type);

    // Json server methods
    QList<QByteArray> processData(const QByteArray &data) override;

    // This method will be called from the proxy server once the client is
    // registered correctly as server or client connection and is now active
    void activateClient();

signals:
    void typeChanged(Type type);

private:
    QTimer *m_inactiveTimer = nullptr;
    Type m_type = TypeNone;

};

QDebug operator<< (QDebug debug, TunnelProxyClient *tunnelProxyClient);

}

#endif // TUNNELPROXYCLIENT_H
