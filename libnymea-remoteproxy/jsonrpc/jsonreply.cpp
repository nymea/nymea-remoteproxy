/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2023, nymea GmbH
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
