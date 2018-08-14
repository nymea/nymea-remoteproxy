#include "authenticationreply.h"
#include "authentication/authenticator.h"

namespace remoteproxy {

AuthenticationReply::AuthenticationReply(ProxyClient *proxyClient, QObject *parent) :
    QObject(parent),
    m_proxyClient(proxyClient)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10000);
    m_timer->setSingleShot(true);

    m_process = new QProcess(this);
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
    m_timer->stop();
    // emit in next event loop
    QTimer::singleShot(0, this, &AuthenticationReply::finished);
}

void AuthenticationReply::onTimeout()
{
    m_timedOut = true;
    m_error = Authenticator::AuthenticationErrorTimeout;
    m_timer->stop();
    m_process->kill();
    emit finished();
}

void AuthenticationReply::abort()
{
    m_error = Authenticator::AuthenticationErrorAborted;
    m_timer->stop();
    m_process->kill();
    emit finished();
}

}
