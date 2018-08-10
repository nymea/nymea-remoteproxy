#include "proxyclient.h"
#include "awsauthenticator.h"
#include "loggingcategories.h"

namespace remoteproxy {

AwsAuthenticator::AwsAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

QString AwsAuthenticator::name() const
{
    return "AWS authenticator";
}

AuthenticationReply *AwsAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthenticator()) << name() << "Start authenticating" <<  proxyClient << "using token" << proxyClient->token();
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, this);
    return reply;
}

}
