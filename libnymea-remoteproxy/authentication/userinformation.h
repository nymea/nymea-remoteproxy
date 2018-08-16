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

#ifndef USERINFORMATION_H
#define USERINFORMATION_H

#include <QDebug>
#include <QString>

namespace remoteproxy {


class UserInformation
{
public:
    UserInformation(const QString &email = QString(), const QString &cognitoUsername  = QString(), const QString &vendorId = QString(), const QString &userPoolId = QString());

    QString email() const;
    QString cognitoUsername() const;
    QString vendorId() const;
    QString userPoolId() const;

    bool isValid();

private:
    QString m_email;
    QString m_cognitoUsername;
    QString m_vendorId;
    QString m_userPoolId;
};

QDebug operator<<(QDebug debug, const UserInformation &userInformation);

}

#endif // USERINFORMATION_H
