#include "proxyclient.h"
#include "awsauthenticator.h"
#include "loggingcategories.h"

namespace remoteproxy {

AwsAuthenticator::AwsAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

AwsAuthenticator::~AwsAuthenticator()
{
    qCDebug(dcAuthenticator()) << "Shutting down" << name();
}

QString AwsAuthenticator::name() const
{
    return "AWS authenticator";
}

AuthenticationReply *AwsAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthenticator()) << name() << "Start authenticating" <<  proxyClient << "using token" << proxyClient->token();
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, this);

    // TODO: start authentication request

    return reply;
}

}
