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

#ifndef JSONREPLY_H
#define JSONREPLY_H

#include <QObject>
#include <QVariantMap>

namespace remoteproxyclient {

class JsonReply : public QObject
{
    Q_OBJECT
public:
    explicit JsonReply(int commandId, QString nameSpace, QString method, QVariantMap params = QVariantMap(), QObject *parent = nullptr);

    int commandId() const;
    QString nameSpace() const;
    QString method() const;
    QVariantMap params() const;
    QVariantMap requestMap();

    QVariantMap response() const;
    void setResponse(const QVariantMap &response);

private:
    int m_commandId;
    QString m_nameSpace;
    QString m_method;
    QVariantMap m_params;
    QVariantMap m_response;

signals:
    void finished();

};

}

#endif // JSONREPLY_H
