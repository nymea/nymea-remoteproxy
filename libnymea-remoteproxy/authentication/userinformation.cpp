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

}
