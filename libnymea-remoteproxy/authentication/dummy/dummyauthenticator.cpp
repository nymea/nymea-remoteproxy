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

#include "dummyauthenticator.h"
#include "loggingcategories.h"

#include <QTimer>

namespace remoteproxy {

DummyAuthenticator::DummyAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

QString DummyAuthenticator::name() const
{
    return "Dummy authenticator";
}

AuthenticationReply *DummyAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthentication()) << name() << "validate" << proxyClient;
    qCWarning(dcAuthentication()) << "Attention: This authenticator will always succeed! This is a security risk and was enabled explitly!";
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, this);

    setReplyError(reply, AuthenticationErrorNoError);
    setReplyFinished(reply);
    return reply;
}

}
