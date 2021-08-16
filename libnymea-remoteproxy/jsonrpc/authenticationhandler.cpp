/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "jsontypes.h"
#include "loggingcategories.h"
#include "authenticationhandler.h"
#include "server/transportclient.h"

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
    params.insert("o:nonce", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("Authenticate", params);
    returns.insert("authenticationError", JsonTypes::authenticationErrorRef());
    setReturns("Authenticate", returns);
}

QString AuthenticationHandler::name() const
{
    return "Authentication";
}

JsonReply *AuthenticationHandler::Authenticate(const QVariantMap &params, TransportClient *transportClient)
{
    QUuid uuid = params.value("uuid").toUuid();
    QString name = params.value("name").toString();
    QString token = params.value("token").toString();
    QString nonce = params.value("nonce").toString();

    qCDebug(dcJsonRpc()) << "Authenticate:" << name << uuid << token << nonce;
    JsonReply *jsonReply = createAsyncReply("Authenticate");

    // Set the token for this proxy client
    ProxyClient *proxyClient = qobject_cast<ProxyClient *>(transportClient);
    proxyClient->setUuid(uuid);
    proxyClient->setName(name);
    proxyClient->setToken(token);
    proxyClient->setNonce(nonce);

    AuthenticationReply *authReply = Engine::instance()->authenticator()->authenticate(proxyClient);
    connect(authReply, &AuthenticationReply::finished, this, &AuthenticationHandler::onAuthenticationFinished);

    m_runningAuthentications.insert(authReply, jsonReply);

    return jsonReply;
}

void AuthenticationHandler::onAuthenticationFinished()
{
    AuthenticationReply *authenticationReply = static_cast<AuthenticationReply *>(sender());
    authenticationReply->deleteLater();

    qCDebug(dcJsonRpc()) << "Authentication reply finished";
    JsonReply *jsonReply = m_runningAuthentications.take(authenticationReply);

    if (authenticationReply->error() != Authenticator::AuthenticationErrorNoError) {
        qCWarning(dcJsonRpc()) << "Authentication error occurred" << authenticationReply->error();
        jsonReply->setSuccess(false);
    } else {
        // Successfully authenticated
        jsonReply->setSuccess(true);
    }

    // Set client authenticated if still there
    if (!authenticationReply->proxyClient().isNull()) {
        authenticationReply->proxyClient()->setAuthenticated(authenticationReply->error() == Authenticator::AuthenticationErrorNoError);
        jsonReply->setData(errorToReply(authenticationReply->error()));
    }

    emit jsonReply->finished();
}

}
