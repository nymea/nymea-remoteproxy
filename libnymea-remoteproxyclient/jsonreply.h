/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2023, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
