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

#include "jsonreply.h"

namespace remoteproxyclient {

JsonReply::JsonReply(int commandId, QString nameSpace, QString method, QVariantMap params, QObject *parent) :
    QObject(parent),
    m_commandId(commandId),
    m_nameSpace(nameSpace),
    m_method(method),
    m_params(params)
{

}

int JsonReply::commandId() const
{
    return m_commandId;
}

QString JsonReply::nameSpace() const
{
    return m_nameSpace;
}

QString JsonReply::method() const
{
    return m_method;
}

QVariantMap JsonReply::params() const
{
    return m_params;
}

QVariantMap JsonReply::requestMap()
{
    QVariantMap request;
    request.insert("id", commandId());
    request.insert("method", nameSpace() + "." + method());
    if (!m_params.isEmpty())
        request.insert("params", params());

    m_commandId++;
    return request;
}

QVariantMap JsonReply::response() const
{
    return m_response;
}

void JsonReply::setResponse(const QVariantMap &response)
{
    m_response = response;
}

}
