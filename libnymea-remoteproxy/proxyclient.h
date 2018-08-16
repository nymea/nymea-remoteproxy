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

#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QUuid>
#include <QDebug>
#include <QObject>

#include "transportinterface.h"

namespace remoteproxy {

class ProxyClient : public QObject
{
    Q_OBJECT

public:
    explicit ProxyClient(TransportInterface *interface, const QUuid &clientId, QObject *parent = nullptr);

    QUuid clientId() const;

    bool isAuthenticated() const;
    void setAuthenticated(bool isAuthenticated);

    bool isTunnelConnected() const;
    void setTunnelConnected(bool isTunnelConnected);

    TransportInterface *interface() const;

    // Properties from auth request
    QString uuid() const;
    void setUuid(const QString &uuid);

    QString name() const;
    void setName(const QString &name);

    QString token() const;
    void setToken(const QString &token);

private:
    TransportInterface *m_interface = nullptr;
    QUuid m_clientId;
    bool m_authenticated = false;
    bool m_tunnelConnected = false;
    QString m_uuid;
    QString m_name;
    QString m_token;

signals:
    void authenticated();
    void tunnelConnected();

};

QDebug operator<< (QDebug debug, ProxyClient *proxyClient);

}

#endif // PROXYCLIENT_H
