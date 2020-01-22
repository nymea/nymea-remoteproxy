/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
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

#include "engine.h"
#include "authenticationreply.h"
#include "authentication/authenticator.h"

namespace remoteproxy {

AuthenticationReply::AuthenticationReply(ProxyClient *proxyClient, QObject *parent) :
    QObject(parent),
    m_proxyClient(proxyClient)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &AuthenticationReply::onTimeout);

    m_timer->start(Engine::instance()->configuration()->authenticationTimeout());
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
    setFinished();
}

void AuthenticationReply::abort()
{
    m_error = Authenticator::AuthenticationErrorAborted;
    setFinished();
}

}
