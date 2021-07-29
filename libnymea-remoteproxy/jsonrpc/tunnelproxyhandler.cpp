/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2021, nymea GmbH
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

#include "tunnelproxyhandler.h"

#include "engine.h"
#include "jsontypes.h"
#include "loggingcategories.h"

#include "tunnelproxy/tunnelproxyserver.h"

namespace remoteproxy {

TunnelProxyHandler::TunnelProxyHandler(QObject *parent) : JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("RegisterServer", "Register a new TunnelProxy server on this instance. Multiple TunnelProxy clients can be connected to the registered server on success.");
    params.insert("serverName", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("serverUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("RegisterServer", params);
    returns.insert("tunnelProxyError", JsonTypes::tunnelProxyErrorRef());
    setReturns("RegisterServer", returns);

    params.clear(); returns.clear();
    setDescription("RegisterClient", "Register a new TunnelProxy client on TunnelProxy server with the given serverUuid..");
    params.insert("clientName", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("clientUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("serverUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("RegisterClient", params);
    returns.insert("tunnelProxyError", JsonTypes::tunnelProxyErrorRef());
    setReturns("RegisterClient", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("ClientConnected", "Emitted whenever a new client has been connected to a registered server. "
                   "Only tunnel proxy clients registered as server will receive this notification.");
    params.insert("clientId", JsonTypes::basicTypeToString(JsonTypes::UInt));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("address", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("ClientConnected", params);

    params.clear(); returns.clear();
    setDescription("ClientDisconnected", "Emitted whenever a new client has been connected to a registered server. "
                   "Only tunnel proxy clients registered as server will receive this notification.");
    params.insert("clientId", JsonTypes::basicTypeToString(JsonTypes::UInt));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("address", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("ClientDisconnected", params);

}

QString TunnelProxyHandler::name() const
{
    return "TunnelProxy";
}

JsonReply *TunnelProxyHandler::RegisterServer(const QVariantMap &params, TransportClient *transportClient)
{
    qCDebug(dcJsonRpc()) << name() << "register server" << params << transportClient;
    QUuid serverUuid = params.value("serverUuid").toUuid();
    TunnelProxyServer::TunnelProxyError error = TunnelProxyServer::TunnelProxyErrorNoError;
    if (serverUuid.isNull()) {
        qCWarning(dcJsonRpc()) << "Invalid uuid received" << params.value("serverUuid").toString() << serverUuid;
        error = TunnelProxyServer::TunnelProxyErrorInvalidUuid;
    } else {
        QString serverName = params.value("serverName").toString();
        error = Engine::instance()->tunnelProxyServer()->registerServer(transportClient->clientId(), serverUuid, serverName);
    }

    QVariantMap response;
    response.insert("tunnelProxyError", JsonTypes::tunnelProxyErrorToString(error));
    return createReply("RegisterServer", response);
}

JsonReply *TunnelProxyHandler::RegisterClient(const QVariantMap &params, TransportClient *transportClient)
{
    qCDebug(dcJsonRpc()) << name() << "register client" << params << transportClient;
    QString clientName = params.value("clientName").toString();
    QUuid clientUuid = params.value("clientUuid").toUuid();
    QUuid serverUuid = params.value("serverUuid").toUuid();
    TunnelProxyServer::TunnelProxyError error = TunnelProxyServer::TunnelProxyErrorNoError;
    if (serverUuid.isNull()) {
        qCWarning(dcJsonRpc()) << "Invalid server uuid received" << params.value("serverUuid").toString() << serverUuid;
        error = TunnelProxyServer::TunnelProxyErrorInvalidUuid;
    } else if (clientUuid.isNull()) {
        qCWarning(dcJsonRpc()) << "Invalid client uuid received" << params.value("clientUuid").toString() << clientUuid;
        error = TunnelProxyServer::TunnelProxyErrorInvalidUuid;
    } else {
        error = Engine::instance()->tunnelProxyServer()->registerClient(transportClient->clientId(), clientUuid, clientName, serverUuid);
    }

    QVariantMap response;
    response.insert("tunnelProxyError", JsonTypes::tunnelProxyErrorToString(error));
    return createReply("RegisterServer", response);
}

//JsonReply *TunnelProxyHandler::RemoveClient(const QVariantMap &params, TransportClient *transportClient)
//{

//}

}
