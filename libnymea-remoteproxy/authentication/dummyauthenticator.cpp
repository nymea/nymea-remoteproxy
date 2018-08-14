#include "dummyauthenticator.h"
#include "loggingcategories.h"

#include <QTimer>

namespace remoteproxy {

DummyAuthenticator::DummyAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

QString DummyAuthenticator::name() const
{
    return "Dummy authenticator";
}

AuthenticationReply *DummyAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthentication()) << name() << "validate" << proxyClient;
    qCWarning(dcAuthentication()) << "Attention: This authenticator will always succeed! This is a security risk and was enabled explitly!";
    AuthenticationReply *reply = createAuthenticationReply(proxyClient, this);

    setReplyError(reply, AuthenticationErrorNoError);
    setReplyFinished(reply);
    return reply;
}

}
