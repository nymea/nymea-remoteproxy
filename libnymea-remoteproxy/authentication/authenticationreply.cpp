#include "authenticationreply.h"
#include "authentication/authenticator.h"

namespace remoteproxy {

AuthenticationReply::AuthenticationReply(ProxyClient *proxyClient, QObject *parent) :
    QObject(parent),
    m_proxyClient(proxyClient)
{

}

ProxyClient *AuthenticationReply::proxyClient() const
{
    return m_proxyClient;
}

bool AuthenticationReply::isTimedOut() const
{
    return m_timedOut;
}

bool AuthenticationReply::isFinished() const
{
    return m_finished;
}

Authenticator::AuthenticationError AuthenticationReply::error() const
{
    return m_error;
}

void AuthenticationReply::setError(Authenticator::AuthenticationError error)
{
    m_error = error;
}

void AuthenticationReply::setFinished()
{
    emit finished();
}

void AuthenticationReply::onTimeout()
{
    m_timedOut = true;
    m_error = Authenticator::AuthenticationErrorTimeout;
    emit finished();
}

void AuthenticationReply::abort()
{
    m_error = Authenticator::AuthenticationErrorAborted;
    emit finished();
}

}
