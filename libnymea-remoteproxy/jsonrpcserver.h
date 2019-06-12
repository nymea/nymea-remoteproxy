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

#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QObject>
#include <QVariant>

#include "proxyclient.h"
#include "jsonrpc/jsonreply.h"
#include "transportinterface.h"
#include "jsonrpc/jsonhandler.h"
#include "jsonrpc/authenticationhandler.h"

namespace remoteproxy {

class JsonRpcServer : public JsonHandler
{
    Q_OBJECT
public:
    explicit JsonRpcServer(QObject *parent = nullptr);
    ~JsonRpcServer() override;

    QString name() const override;

    Q_INVOKABLE JsonReply *Hello(const QVariantMap &params, ProxyClient *proxyClient = nullptr) const;
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params, ProxyClient *proxyClient = nullptr) const;

signals:
    void TunnelEstablished(const QVariantMap &params);

private:
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, ProxyClient *> m_asyncReplies;
    QList<ProxyClient *> m_clients;

    int m_notificationId = 0;

    void sendResponse(ProxyClient *client, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(ProxyClient *client, int commandId, const QString &error);

    QString formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const;

    void registerHandler(JsonHandler *handler);
    void unregisterHandler(JsonHandler *handler);
    void processDataPackage(ProxyClient *proxyClient, const QByteArray &data);

private slots:
    void setup();
    void asyncReplyFinished();

public slots:
    // Client registration for JSON RPC traffic
    void registerClient(ProxyClient *proxyClient);
    void unregisterClient(ProxyClient *proxyClient);

    // Process data from client
    void processData(ProxyClient *proxyClient, const QByteArray &data);
    void sendNotification(const QString &nameSpace, const QString &method, const QVariantMap &params, ProxyClient *proxyClient = nullptr);

};

}

#endif // JSONRPCSERVER_H
