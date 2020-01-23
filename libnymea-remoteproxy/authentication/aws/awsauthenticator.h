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

#ifndef AWSAUTHENTICATOR_H
#define AWSAUTHENTICATOR_H

#include <QObject>
#include <QNetworkAccessManager>

#include "awscredentialprovider.h"
#include "authenticationprocess.h"
#include "authentication/authenticator.h"
#include "authentication/authenticationreply.h"

namespace remoteproxy {

class AwsAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit AwsAuthenticator(const QUrl &awsCredentialsUrl, QObject *parent = nullptr);
    ~AwsAuthenticator() override;

    QString name() const override;

private:
    QNetworkAccessManager *m_manager = nullptr;
    AwsCredentialProvider *m_credentialsProvider = nullptr;
    QHash<AuthenticationProcess *, AuthenticationReply *> m_runningProcesses;

private slots:
    void onAuthenticationProcessFinished(Authenticator::AuthenticationError error, const UserInformation &userInformation);

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;

};

}

#endif // AWSAUTHENTICATOR_H
