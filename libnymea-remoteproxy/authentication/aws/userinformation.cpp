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
