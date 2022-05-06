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
#include "proxy/proxyclient.h"
#include "awsauthenticator.h"
#include "loggingcategories.h"

namespace remoteproxy {

AwsAuthenticator::AwsAuthenticator(const QUrl &awsCredentialsUrl, QObject *parent) :
    Authenticator(parent),
    m_manager(new QNetworkAccessManager(this))
{
    m_credentialsProvider = new AwsCredentialProvider(m_manager, awsCredentialsUrl, this);
    QMetaObject::invokeMethod(m_credentialsProvider, QString("enable").toLatin1().data(), Qt::QueuedConnection);
}

AwsAuthenticator::~AwsAuthenticator()
{
    qCDebug(dcAuthentication()) << "Shutting down" << name();
    m_credentialsProvider->disable();
}

QString AwsAuthenticator::name() const
{
    return "AWS authenticator";
}

AuthenticationReply *AwsAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthentication()) << name() << "Start authenticating" << proxyClient;
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, proxyClient);
    if (!m_credentialsProvider->isValid()) {
        qCWarning(dcAuthentication()) << name() << "There are no credentials for authenticating.";
        setReplyError(reply, AuthenticationErrorProxyError);
        setReplyFinished(reply);
        return reply;
    }

    AuthenticationProcess *process = new AuthenticationProcess(m_manager,
                                                               m_credentialsProvider->accessKey(),
                                                               m_credentialsProvider->secretAccessKey(),
                                                               m_credentialsProvider->sessionToken(), reply);

    connect(process, &AuthenticationProcess::authenticationFinished, proxyClient, [=](Authenticator::AuthenticationError error, const UserInformation &userInformation = UserInformation()){
        if (error == AuthenticationErrorNoError) {
            qCDebug(dcAuthentication()) << name() << proxyClient << "finished successfully." << userInformation;
        } else {
            qCDebug(dcAuthentication()) << name() << proxyClient << "finished with error" << error;
        }

        proxyClient->setUserName(userInformation.email());

        setReplyError(reply, error);
        setReplyFinished(reply);
    });


    // Start authentication process
    process->authenticate(proxyClient->token());
    return reply;
}

}
