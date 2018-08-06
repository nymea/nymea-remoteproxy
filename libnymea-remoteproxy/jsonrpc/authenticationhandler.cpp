#include "jsontypes.h"
#include "loggingcategories.h"
#include "authenticationhandler.h"

#include "engine.h"

AuthenticationHandler::AuthenticationHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;

    setDescription("Authenticate", "Authenticate this connection. This should always be the first request to the server. The given id is the unique server/client uuid (i.e. the uuid of server/client).");
    params.insert("id", JsonTypes::basicTypeToString(JsonTypes::Uuid));
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

JsonReply *AuthenticationHandler::Authenticate(const QVariantMap &params, const QUuid &clientId)
{
    qCDebug(dcJsonRpc()) << "Authenticate:" << clientId.toString();

    QString clientName = params.value("name").toString();
    QString clientToken = params.value("token").toString();
    QUuid clientDeviceId = QUuid(params.value("id").toString());

    Q_UNUSED(clientDeviceId);

    JsonReply *jsonReply = createAsyncReply("Authenticate");
    AuthenticationReply *authReply = Engine::instance()->authenticator()->authenticate(clientId, clientToken);
    connect(authReply, &AuthenticationReply::finished, this, &AuthenticationHandler::onAuthenticationFinished);

    m_runningAuthentications.insert(authReply, jsonReply);

    return jsonReply;
}

void AuthenticationHandler::onAuthenticationFinished()
{
    //AuthenticationReply *authReply = static_cast<AuthenticationReply *>(sender());
    //JsonReply *jsonReply = m_runningAuthentications.take(authReply);
    
    //emit asyncReply()
}
