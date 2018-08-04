#include "jsonhandler.h"
#include <QMetaMethod>
#include <QDebug>
#include <QRegExp>

#include "jsonreply.h"
#include "jsontypes.h"
#include "loggingcategories.h"

JsonHandler::JsonHandler(QObject *parent):
    QObject(parent)
{

}

QVariantMap JsonHandler::introspect(const QMetaMethod::MethodType &type)
{
    QVariantMap data;
    for (int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);

        if (method.methodType() != type)
            continue;

        switch (method.methodType()) {
        case QMetaMethod::Method: {
            if (!m_descriptions.contains(method.name()) || !m_params.contains(method.name()) || !m_returns.contains(method.name())) {
                continue;
            }
            QVariantMap methodData;
            methodData.insert("description", m_descriptions.value(method.name()));
            methodData.insert("params", m_params.value(method.name()));
            methodData.insert("returns", m_returns.value(method.name()));
            data.insert(name() + "." + method.name(), methodData);
            break;
        }
        case QMetaMethod::Signal: {
            if (!m_descriptions.contains(method.name()) || !m_params.contains(method.name())) {
                continue;
            }
            if (QString(method.name()).contains(QRegExp("^[A-Z]"))) {
                QVariantMap methodData;
                methodData.insert("description", m_descriptions.value(method.name()));
                methodData.insert("params", m_params.value(method.name()));
                data.insert(name() + "." + method.name(), methodData);
            }
            break;
        default:
            ;;// Nothing to do for slots
        }
        }
    }
    return data;
}

bool JsonHandler::hasMethod(const QString &methodName)
{
    return m_descriptions.contains(methodName) && m_params.contains(methodName) && m_returns.contains(methodName);
}

QPair<bool, QString> JsonHandler::validateParams(const QString &methodName, const QVariantMap &params)
{
    QVariantMap paramTemplate = m_params.value(methodName);
    return JsonTypes::validateMap(paramTemplate, params);
}

QPair<bool, QString> JsonHandler::validateReturns(const QString &methodName, const QVariantMap &returns)
{
    QVariantMap returnsTemplate = m_returns.value(methodName);
    return JsonTypes::validateMap(returnsTemplate, returns);
}

void JsonHandler::setDescription(const QString &methodName, const QString &description)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_descriptions.insert(methodName, description);
            return;
        }
    }
    qCWarning(dcJsonRpc()) << "Cannot set description. No such method:" << methodName;
}

void JsonHandler::setParams(const QString &methodName, const QVariantMap &params)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_params.insert(methodName, params);
            return;
        }
    }
    qCWarning(dcJsonRpc()) << "Cannot set params. No such method:" << methodName;
}

void JsonHandler::setReturns(const QString &methodName, const QVariantMap &returns)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_returns.insert(methodName, returns);
            return;
        }
    }
    qCWarning(dcJsonRpc()) << "Cannot set returns. No such method:" << methodName;
}

JsonReply *JsonHandler::createReply(const QVariantMap &data) const
{
    return JsonReply::createReply(const_cast<JsonHandler*>(this), data);
}

JsonReply *JsonHandler::createAsyncReply(const QString &method) const
{
    return JsonReply::createAsyncReply(const_cast<JsonHandler*>(this), method);
}
