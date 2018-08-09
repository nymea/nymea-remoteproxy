#include "proxyclient.h"
#include "authenticator.h"
#include "authenticationreply.h"

namespace remoteproxy {

Authenticator::Authenticator(QObject *parent) :
    QObject(parent)
{

}

void Authenticator::setReplyError(AuthenticationReply *reply, Authenticator::AuthenticationError error)
{
    reply->setError(error);
}

void Authenticator::setReplyFinished(AuthenticationReply *reply)
{
    reply->setFinished();
}

AuthenticationReply *Authenticator::createAuthenticationReply(ProxyClient *proxyClient, QObject *parent)
{
    return new AuthenticationReply(proxyClient, parent);
}

Authenticator::~Authenticator()
{

}

}
