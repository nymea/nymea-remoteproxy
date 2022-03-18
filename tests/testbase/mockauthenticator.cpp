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

#include "proxy/proxyclient.h"
#include "mockauthenticator.h"
#include "loggingcategories.h"
#include "authentication/authenticationreply.h"

MockAuthenticator::MockAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

QString MockAuthenticator::name() const
{
    return "Mock authenticator";
}

void MockAuthenticator::setTimeoutDuration(int timeout)
{
    m_timeoutDuration = timeout;
}

void MockAuthenticator::setExpectedAuthenticationError(Authenticator::AuthenticationError error)
{
    m_expectedError = error;
}

void MockAuthenticator::replyFinished()
{
    MockAuthenticationReply *reply = static_cast<MockAuthenticationReply *>(sender());

    qCDebug(dcAuthentication()) << name() << "Authentication finished" << reply << reply->authenticationReply();
    setReplyError(reply->authenticationReply(), reply->error());
    setReplyFinished(reply->authenticationReply());
    reply->deleteLater();
}

AuthenticationReply *MockAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthentication()) << name() << "Start authentication for" << proxyClient << "using token" << proxyClient->token() << "Auth duration" << m_timeoutDuration << "[ms]";

    AuthenticationReply *authenticationReply = createAuthenticationReply(proxyClient, proxyClient);

    // Create mock authentication reply with the given configuration
    MockAuthenticationReply *reply = new MockAuthenticationReply(m_timeoutDuration, m_expectedError, authenticationReply, proxyClient);
    connect(reply, &MockAuthenticationReply::finished, this, &MockAuthenticator::replyFinished);

    return authenticationReply;
}

MockAuthenticationReply::MockAuthenticationReply(int timeout, Authenticator::AuthenticationError error, AuthenticationReply *authenticationReply, QObject *parent) :
    QObject(parent),
    m_error(error),
    m_authenticationReply(authenticationReply)

{
    QTimer::singleShot(timeout, this, &MockAuthenticationReply::finished);
}

MockAuthenticationReply::~MockAuthenticationReply()
{

}

