#ifndef USERINFORMATION_H
#define USERINFORMATION_H

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

}

#endif // USERINFORMATION_H
