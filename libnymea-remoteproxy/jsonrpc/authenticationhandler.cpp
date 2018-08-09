#include "jsontypes.h"
#include "loggingcategories.h"
#include "authenticationhandler.h"

#include "engine.h"

namespace remoteproxy {

AuthenticationHandler::AuthenticationHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;

    setDescription("Authenticate", "Authenticate this connection. The returned AuthenticationError informs "
                   "about the result. If the authentication was not successfull, the server will close the "
                   "connection immediatly after sending the error response. The given id should be a unique "
                   "id the other tunnel client can understand. Once the authentication was successfull, you "
                   "can wait for the RemoteProxy.TunnelEstablished notification. If you send any data before "
                   "getting this notification, the server will close the connection. If the tunnel client does "
                   "not show up within 10 seconds, the server will close the connection.");
    params.insert("uuid", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("token", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("Authenticate", params);
    returns.insert("authenticationError", JsonTypes::authenticationErrorRef());
    setReturns("Authenticate", returns);
}

QString AuthenticationHandler::name() const
{
    return "Authentication";
}

JsonReply *AuthenticationHandler::Authenticate(const QVariantMap &params, ProxyClient *proxyClient)
{
    QString uuid = params.value("uuid").toString();
    QString name = params.value("name").toString();
    QString token = params.value("token").toString();

    qCDebug(dcJsonRpc()) << "Authenticate:" << name << uuid << token;
    JsonReply *jsonReply = createAsyncReply("Authenticate");

    // Set the token for this proxy client
    proxyClient->setUuid(uuid);
    proxyClient->setName(name);
    proxyClient->setToken(token);

    AuthenticationReply *authReply = Engine::instance()->authenticator()->authenticate(proxyClient);
    connect(authReply, &AuthenticationReply::finished, this, &AuthenticationHandler::onAuthenticationFinished);

    m_runningAuthentications.insert(authReply, jsonReply);

    return jsonReply;
}

void AuthenticationHandler::onAuthenticationFinished()
{
    AuthenticationReply *authenticationReply = static_cast<AuthenticationReply *>(sender());
    JsonReply *jsonReply = m_runningAuthentications.take(authenticationReply);

    authenticationReply->deleteLater();

    qCDebug(dcJsonRpc()) << "Authentication respons ready for" << authenticationReply->proxyClient() << authenticationReply->error();

    if (authenticationReply->error() != Authenticator::AuthenticationErrorNoError) {
        qCWarning(dcJsonRpc()) << "Authentication error occured" << authenticationReply->error();
        jsonReply->setSuccess(true);
        authenticationReply->proxyClient()->setAuthenticated(true);
    } else {
        authenticationReply->proxyClient()->setAuthenticated(false);
        jsonReply->setSuccess(false);
    }
    
    jsonReply->setData(errorToReply(authenticationReply->error()));
    jsonReply->finished();
}

}
