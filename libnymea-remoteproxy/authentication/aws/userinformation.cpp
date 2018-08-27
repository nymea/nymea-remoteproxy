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

#include "userinformation.h"

namespace remoteproxy {

UserInformation::UserInformation(const QString &email, const QString &cognitoUsername, const QString &vendorId, const QString &userPoolId) :
    m_email(email),
    m_cognitoUsername(cognitoUsername),
    m_vendorId(vendorId),
    m_userPoolId(userPoolId)
{

}

QString UserInformation::email() const
{
    return m_email;
}

QString UserInformation::cognitoUsername() const
{
    return m_cognitoUsername;
}

QString UserInformation::vendorId() const
{
    return  m_vendorId;
}

QString UserInformation::userPoolId() const
{
    return m_userPoolId;
}

bool UserInformation::isValid()
{
    return !m_email.isEmpty()
            && !m_cognitoUsername.isEmpty()
            && !m_vendorId.isEmpty()
            && !m_userPoolId.isEmpty();
}

QDebug operator<<(QDebug debug, const UserInformation &userInformation)
{
    debug.nospace() << "UserInformation(" << userInformation.email();
    debug.nospace() << ", cognito:" << userInformation.cognitoUsername() << ") ";
    debug.nospace() << ", vendor" << userInformation.vendorId() << ") ";
    debug.nospace() << ", userpool" << userInformation.userPoolId() << ") ";
    return debug;
}

}
