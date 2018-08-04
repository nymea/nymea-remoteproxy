#include "mockauthenticator.h"
#include "loggingcategories.h"
#include "authentication/authenticationreply.h"


MockAuthenticator::MockAuthenticator(QObject *parent) :
    Authenticator(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);

    connect(m_timer, &QTimer::timeout, this, &MockAuthenticator::onTimeout);
}

void MockAuthenticator::onTimeout()
{

}

AuthenticationReply *MockAuthenticator::authenticate(const QUuid &clientId, const QString &token)
{
    qCDebug(dcAuthenticator()) << "MockAuthenticator: Start authentication for" << clientId << "using token" << token;
    AuthenticationReply *reply = new AuthenticationReply(clientId, token, this);
    return reply;
}
