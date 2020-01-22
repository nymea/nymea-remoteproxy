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

#ifndef MOCKAUTHENTICATOR_H
#define MOCKAUTHENTICATOR_H

#include <QTimer>
#include <QObject>

#include "authentication/authenticator.h"

using namespace remoteproxy;

class MockAuthenticationReply : public QObject
{
    Q_OBJECT
public:
    explicit MockAuthenticationReply(int timeout, Authenticator::AuthenticationError error, AuthenticationReply *authenticationReply, QObject *parent = nullptr);

    AuthenticationReply *authenticationReply() const { return m_authenticationReply; }
    Authenticator::AuthenticationError error() const { return m_error; }

private:
    Authenticator::AuthenticationError m_error;
    AuthenticationReply *m_authenticationReply = nullptr;

signals:
    void finished();

};


class MockAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit MockAuthenticator(QObject *parent = nullptr);

    QString name() const override;

    void setTimeoutDuration(int timeout);
    void setExpectedAuthenticationError(Authenticator::AuthenticationError error = AuthenticationErrorNoError);

private:
    int m_timeoutDuration = 1000;
    Authenticator::AuthenticationError m_expectedError;

private slots:
    void replyFinished();

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;
};

#endif // MOCKAUTHENTICATOR_H
