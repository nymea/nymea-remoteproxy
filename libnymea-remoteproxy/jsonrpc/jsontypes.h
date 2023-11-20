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

#ifndef JSONTYPES_H
#define JSONTYPES_H

#include <QObject>
#include <QVariant>
#include <QMetaEnum>
#include <QStringList>

#include "tunnelproxy/tunnelproxyserver.h"

namespace remoteproxy {

#define DECLARE_OBJECT(typeName, jsonName) \
    public: \
    static QString typeName##Ref() { return QStringLiteral("$ref:") + QStringLiteral(jsonName); if (!s_initialized) { init(); } } \
    static QVariantMap typeName##Description() { \
        if (!s_initialized) { init(); } \
        return s_##typeName; \
    } \
    private: \
    static QVariantMap s_##typeName; \
    public:

#define DECLARE_TYPE(typeName, enumString, className, enumName) \
    public: \
    static QString typeName##Ref() { return QStringLiteral("$ref:") + QStringLiteral(enumString); if (!s_initialized) { init(); } } \
    static QVariantList typeName() { \
        if (!s_initialized) { init(); } \
        return s_##typeName; \
    } \
    static QString typeName##ToString(className::enumName value) { \
        if (!s_initialized) { init(); } \
        QMetaObject metaObject = className::staticMetaObject; \
        int enumIndex = metaObject.indexOfEnumerator(enumString); \
        QMetaEnum metaEnum = metaObject.enumerator(enumIndex); \
        return metaEnum.valueToKey(metaEnum.value(value)); \
    } \
    private: \
    static QVariantList s_##typeName; \
    public:


class JsonTypes
{
    Q_GADGET

public:
    enum BasicType {
        Uuid,
        String,
        Int,
        UInt,
        Double,
        Bool,
        Variant,
        Object
    };
    Q_ENUM(BasicType)

    static QVariantMap allTypes();

    // Declare types
    DECLARE_TYPE(basicType, "BasicType", JsonTypes, BasicType)
    DECLARE_TYPE(tunnelProxyError, "TunnelProxyError", TunnelProxyServer, TunnelProxyError)

    // Declare objects

    // Pack methods

    // Validation methods
    static QPair<bool, QString> validateMap(const QVariantMap &templateMap, const QVariantMap &map);
    static QPair<bool, QString> validateVariant(const QVariant &templateVariant, const QVariant &variant);
    static QPair<bool, QString> validateEnum(const QVariantList &enumList, const QVariant &value);
    static QPair<bool, QString> validateProperty(const QVariant &templateValue, const QVariant &value);
    static QPair<bool, QString> validateList(const QVariantList &templateList, const QVariantList &list);
    static QPair<bool, QString> validateBasicType(const QVariant &variant);

    // Converter
    static QString basicTypeToString(const QVariant::Type &type);

private:
    static bool s_initialized;
    static QString s_lastError;

    static void init();

    static QPair<bool, QString> report(bool status, const QString &message);
    static QVariantList enumToStrings(const QMetaObject &metaObject, const QString &enumName);
};

}

#endif // JSONTYPES_H
