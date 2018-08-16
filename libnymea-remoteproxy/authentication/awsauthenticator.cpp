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

#include "engine.h"
#include "proxyclient.h"
#include "awsauthenticator.h"
#include "loggingcategories.h"

namespace remoteproxy {

AwsAuthenticator::AwsAuthenticator(QObject *parent) :
    Authenticator(parent),
    m_manager(new QNetworkAccessManager(this))
{
    // TODO: verify if aws command is installed
}

AwsAuthenticator::~AwsAuthenticator()
{
    qCDebug(dcAuthentication()) << "Shutting down" << name();
}

QString AwsAuthenticator::name() const
{
    return "AWS authenticator";
}

void AwsAuthenticator::onAuthenticationProcessFinished(Authenticator::AuthenticationError error, const UserInformation &userInformation)
{
    AuthenticationProcess *process = static_cast<AuthenticationProcess *>(sender());
    AuthenticationReply *reply = m_runningProcesses.take(process);

    if (error == AuthenticationErrorNoError) {
        qCDebug(dcAuthentication()) << name() << reply->proxyClient() << "finished successfully." << userInformation;
    } else {
        qCDebug(dcAuthentication()) << name() << reply->proxyClient() << "finished with error" << error;
    }

    setReplyError(reply, error);
    setReplyFinished(reply);
}

AuthenticationReply *AwsAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthentication()) << name() << "Start authenticating" << proxyClient;
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, this);

    AuthenticationProcess *process = new AuthenticationProcess(m_manager, this);
    process->useDynamicCredentials(!Engine::instance()->developerMode());
    connect(process, &AuthenticationProcess::authenticationFinished, this, &AwsAuthenticator::onAuthenticationProcessFinished);

    // Configure process
    m_runningProcesses.insert(process, reply);

    // Start authentication process
    process->authenticate(proxyClient->token());
    return reply;
}

}
