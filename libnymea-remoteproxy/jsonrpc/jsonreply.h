/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef JSONRPCREPLY_H
#define JSONRPCREPLY_H

#include <QObject>
#include <QTimer>
#include <QUuid>

#include "jsonhandler.h"

namespace remoteproxy {

class JsonReply: public QObject
{
    Q_OBJECT
public:
    enum Type {
        TypeSync,
        TypeAsync
    };

    friend class JsonRpcServer;

    static JsonReply *createReply(JsonHandler *handler, const QString &method, const QVariantMap &data);
    static JsonReply *createAsyncReply(JsonHandler *handler, const QString &method);

    Type type() const;

    QVariantMap data() const;
    void setData(const QVariantMap &data);

    JsonHandler *handler() const;
    QString method() const;

    QUuid clientId() const;
    void setClientId(const QUuid &clientId);

    int commandId() const;
    void setCommandId(int commandId);

    bool success() const;
    void setSuccess(bool success);

    bool timedOut() const;

public slots:
    void startWait();

signals:
    void finished();

private slots:
    void timeout();

private:
    JsonReply(Type type, JsonHandler *handler, const QString &method, const QVariantMap &data = QVariantMap(), bool success = true);

    Type m_type = TypeSync;
    QVariantMap m_data;

    JsonHandler *m_handler = nullptr;

    QString m_method;
    QUuid m_clientId;
    int m_commandId;
    bool m_timedOut = false;
    bool m_success = false;

    QTimer m_timeout;
};

}

#endif // JSONRPCREPLY_H
