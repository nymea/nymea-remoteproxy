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

#include "jsonhandler.h"
#include <QMetaMethod>
#include <QDebug>
#include <QRegularExpression>

#include "jsonreply.h"
#include "jsontypes.h"
#include "loggingcategories.h"

namespace remoteproxy {

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
            if (QString(method.name()).contains(QRegularExpression("^[A-Z]"))) {
                QVariantMap methodData;
                methodData.insert("description", m_descriptions.value(method.name()));
                methodData.insert("params", m_params.value(method.name()));
                data.insert(name() + "." + method.name(), methodData);
            }
            break;
        default:
            break; // Nothing to do for slots
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
    for (int i = 0; i < metaObject()->methodCount(); ++i) {
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
    for (int i = 0; i < metaObject()->methodCount(); ++i) {
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
    for (int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_returns.insert(methodName, returns);
            return;
        }
    }
    qCWarning(dcJsonRpc()) << "Cannot set returns. No such method:" << methodName;
}


JsonReply *JsonHandler::createReply(const QString &method, const QVariantMap &data) const
{
    return JsonReply::createReply(const_cast<JsonHandler*>(this), method, data);
}

JsonReply *JsonHandler::createAsyncReply(const QString &method) const
{
    return JsonReply::createAsyncReply(const_cast<JsonHandler*>(this), method);
}

}
