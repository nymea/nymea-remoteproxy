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
    request.insert("id", m_commandId);
    request.insert("method", m_nameSpace + "." + m_method);
    if (!m_params.isEmpty())
        request.insert("params", m_params);

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
