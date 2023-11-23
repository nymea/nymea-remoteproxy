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
