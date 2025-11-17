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

#include "engine.h"
#include "jsonreply.h"

namespace remoteproxy {

JsonReply *JsonReply::createReply(JsonHandler *handler, const QString &method, const QVariantMap &data)
{
    return new JsonReply(TypeSync, handler, method, data);
}

JsonReply *JsonReply::createAsyncReply(JsonHandler *handler, const QString &method)
{
    return new JsonReply(TypeAsync, handler, method);
}

JsonReply::Type JsonReply::type() const
{
    return m_type;
}

QVariantMap JsonReply::data() const
{
    return m_data;
}

void JsonReply::setData(const QVariantMap &data)
{
    m_data = data;
}

JsonHandler *JsonReply::handler() const
{
    return m_handler;
}

QString JsonReply::method() const
{
    return m_method;
}

QUuid JsonReply::clientId() const
{
    return m_clientId;
}

void JsonReply::setClientId(const QUuid &clientId)
{
    m_clientId = clientId;
}

int JsonReply::commandId() const
{
    return m_commandId;
}

void JsonReply::setCommandId(int commandId)
{
    m_commandId = commandId;
}

bool JsonReply::success() const
{
    return m_success;
}

void JsonReply::setSuccess(bool success)
{
    m_success = success;
}

bool JsonReply::timedOut() const
{
    return m_timedOut;
}

void JsonReply::startWait()
{
    m_timeout.start(Engine::instance()->configuration()->jsonRpcTimeout());
}

void JsonReply::timeout()
{
    m_timedOut = true;
    emit finished();
}

JsonReply::JsonReply(JsonReply::Type type, JsonHandler *handler, const QString &method, const QVariantMap &data, bool success):
    m_type(type),
    m_data(data),
    m_handler(handler),
    m_method(method),
    m_success(success)
{
    connect(&m_timeout, &QTimer::timeout, this, &JsonReply::timeout);
}



}
