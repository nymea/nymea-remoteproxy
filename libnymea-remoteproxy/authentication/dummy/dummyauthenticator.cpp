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
    qCWarning(dcAuthentication()) << "Attention: This authenticator will always succeed! This is a security risk and was explicitly enabled!";
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, proxyClient);

    proxyClient->setUserName("dummy@example.com");

    setReplyError(reply, AuthenticationErrorNoError);
    setReplyFinished(reply);
    return reply;
}

}
