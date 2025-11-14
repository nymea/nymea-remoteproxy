// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QObject>
#include <QVariant>

#include "transportclient.h"
#include "jsonrpc/jsonreply.h"
#include "transportinterface.h"
#include "jsonrpc/jsonhandler.h"

namespace remoteproxy {

class JsonRpcServer : public JsonHandler
{
    Q_OBJECT
public:
    explicit JsonRpcServer(QObject *parent = nullptr);
    ~JsonRpcServer() override;

    QString name() const override;

    Q_INVOKABLE JsonReply *Hello(const QVariantMap &params, TransportClient *transportClient = nullptr) const;
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params, TransportClient *transportClient = nullptr) const;

    void registerHandler(JsonHandler *handler);
    void unregisterHandler(JsonHandler *handler);

    uint registeredClientCount() const;

signals:
    void TunnelEstablished(const QVariantMap &params);

private:
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, TransportClient *> m_asyncReplies;
    QList<TransportClient *> m_clients;

    int m_notificationId = 0;

    void sendResponse(TransportClient *client, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(TransportClient *client, int commandId, const QString &error);

    QString formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const;


private slots:
    void asyncReplyFinished();

public slots:
    void processDataPacket(TransportClient *transportClient, const QByteArray &data);

    // Client registration for JSON RPC traffic
    void registerClient(TransportClient *transportClient);
    void unregisterClient(TransportClient *transportClient);

    // Process data from client
    void processData(TransportClient *transportClient, const QByteArray &data);
    void sendNotification(const QString &nameSpace, const QString &method, const QVariantMap &params, TransportClient *transportClient = nullptr);

};

}

#endif // JSONRPCSERVER_H
