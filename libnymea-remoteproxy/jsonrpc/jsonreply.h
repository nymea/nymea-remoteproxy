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
