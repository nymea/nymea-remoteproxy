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

#include "authenticationreply.h"
#include "authentication/authenticator.h"

namespace remoteproxy {

AuthenticationReply::AuthenticationReply(ProxyClient *proxyClient, QObject *parent) :
    QObject(parent),
    m_proxyClient(proxyClient)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10000);
    m_timer->setSingleShot(true);

    m_process = new QProcess(this);
}

ProxyClient *AuthenticationReply::proxyClient() const
{
    return m_proxyClient;
}

bool AuthenticationReply::isTimedOut() const
{
    return m_timedOut;
}

bool AuthenticationReply::isFinished() const
{
    return m_finished;
}

Authenticator::AuthenticationError AuthenticationReply::error() const
{
    return m_error;
}

void AuthenticationReply::setError(Authenticator::AuthenticationError error)
{
    m_error = error;
}

void AuthenticationReply::setFinished()
{
    m_timer->stop();
    // emit in next event loop
    QTimer::singleShot(0, this, &AuthenticationReply::finished);
}

void AuthenticationReply::onTimeout()
{
    m_timedOut = true;
    m_error = Authenticator::AuthenticationErrorTimeout;
    m_timer->stop();
    m_process->kill();
    QTimer::singleShot(0, this, &AuthenticationReply::finished);
}

void AuthenticationReply::abort()
{
    m_error = Authenticator::AuthenticationErrorAborted;
    m_timer->stop();
    m_process->kill();
    QTimer::singleShot(0, this, &AuthenticationReply::finished);
}

}
