#include "proxyclient.h"
#include "mockauthenticator.h"
#include "loggingcategories.h"
#include "authentication/authenticationreply.h"

MockAuthenticator::MockAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

QString MockAuthenticator::name() const
{
    return "Mock authenticator";
}

void MockAuthenticator::setTimeoutDuration(int timeout)
{
    m_timeoutDuration = timeout;
}

void MockAuthenticator::setExpectedAuthenticationError(Authenticator::AuthenticationError error)
{
    m_expectedError = error;
}

void MockAuthenticator::replyFinished()
{
    MockAuthenticationReply *reply = static_cast<MockAuthenticationReply *>(sender());
    reply->deleteLater();

    qCDebug(dcAuthenticator()) << name() << "Authentication finished.";

    setReplyError(reply->authenticationReply(), reply->error());
    setReplyFinished(reply->authenticationReply());
}


AuthenticationReply *MockAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthenticator()) << name() << "Start authentication for" << proxyClient << "using token" << proxyClient->token();

    AuthenticationReply *authenticationReply = createAuthenticationReply(proxyClient, this);

    MockAuthenticationReply *reply = new MockAuthenticationReply(m_timeoutDuration, m_expectedError, authenticationReply, this);
    connect(reply, &MockAuthenticationReply::finished, this, &MockAuthenticator::replyFinished);

    return authenticationReply;
}

MockAuthenticationReply::MockAuthenticationReply(int timeout, Authenticator::AuthenticationError error, AuthenticationReply *authenticationReply, QObject *parent) :
    QObject(parent),
    m_error(error),
    m_authenticationReply(authenticationReply)

{
    QTimer::singleShot(timeout, this, &MockAuthenticationReply::finished);
}

