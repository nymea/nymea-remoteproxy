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

#ifndef AUTHENTICATIONHANDLER_H
#define AUTHENTICATIONHANDLER_H

#include <QObject>

#include "jsonhandler.h"
#include "authentication/authenticationreply.h"

namespace remoteproxy {

class AuthenticationHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit AuthenticationHandler(QObject *parent = nullptr);
    ~AuthenticationHandler() override = default;

    QString name() const override;

    Q_INVOKABLE JsonReply *Authenticate(const QVariantMap &params, ProxyClient *proxyClient);

private:
    QHash<AuthenticationReply *, JsonReply *> m_runningAuthentications;

private slots:
    void onAuthenticationFinished();


};

}

#endif // AUTHENTICATIONHANDLER_H
