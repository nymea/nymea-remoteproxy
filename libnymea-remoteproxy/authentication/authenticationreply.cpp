#include "authenticationreply.h"
#include "authentication/authenticator.h"

AuthenticationReply::AuthenticationReply(const QUuid clientId, const QString &token, QObject *parent) :
    QObject(parent),
    m_clientId(clientId),
    m_token(token)
{

}

QUuid AuthenticationReply::clientId() const
{
    return m_clientId;
}

QString AuthenticationReply::token() const
{
    return m_token;
}

bool AuthenticationReply::isTimedOut() const
{
    return m_timedOut;
}

bool AuthenticationReply::isFinished() const
{
    return m_finished;
}

void AuthenticationReply::setError(Authenticator::AuthenticationError error)
{
    m_error = error;
}

void AuthenticationReply::onTimeout()
{


}

void AuthenticationReply::abort()
{

}
