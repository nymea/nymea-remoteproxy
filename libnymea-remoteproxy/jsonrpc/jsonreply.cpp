#include "jsonreply.h"

namespace remoteproxy {

JsonReply *JsonReply::createReply(JsonHandler *handler, const QVariantMap &data)
{
    return new JsonReply(TypeSync, handler, QString(), data);
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
    m_timeout.start(5000);
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
