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

#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QVariantMap>
#include <QMetaMethod>

namespace remoteproxy {

class JsonReply;

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = nullptr);

    virtual QString name() const = 0;

    QVariantMap introspect(const QMetaMethod::MethodType &type);

    bool hasMethod(const QString &methodName);
    QPair<bool, QString> validateParams(const QString &methodName, const QVariantMap &params);
    QPair<bool, QString> validateReturns(const QString &methodName, const QVariantMap &returns);

private:
    QHash<QString, QString> m_descriptions;
    QHash<QString, QVariantMap> m_params;
    QHash<QString, QVariantMap> m_returns;

signals:
    void asyncReply(int id, const QVariantMap &params);

protected:
    void setDescription(const QString &methodName, const QString &description);
    void setParams(const QString &methodName, const QVariantMap &params);
    void setReturns(const QString &methodName, const QVariantMap &returns);

    JsonReply *createReply(const QString &method, const QVariantMap &data) const;
    JsonReply *createAsyncReply(const QString &method) const;
};

}

#endif // JSONHANDLER_H
