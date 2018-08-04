#include "awsauthenticator.h"
#include "loggingcategories.h"

AwsAuthenticator::AwsAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

AuthenticationReply *AwsAuthenticator::authenticate(const QUuid &clientId, const QString &token)
{
    qCDebug(dcAuthenticator()) << "Start authenticating" << clientId << "using token" << token;

    AuthenticationReply *reply = new AuthenticationReply(clientId, token, this);

    return reply;
}
