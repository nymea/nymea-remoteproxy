/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
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

#include "jsontypes.h"
#include <QStringList>
#include <QJsonDocument>
#include <QDebug>

#include "loggingcategories.h"

namespace remoteproxy {

bool JsonTypes::s_initialized = false;
QString JsonTypes::s_lastError;

// Types
QVariantList JsonTypes::s_basicType;
QVariantList JsonTypes::s_authenticationError;
QVariantList JsonTypes::s_tunnelProxyError;

// Objects


QVariantMap JsonTypes::allTypes()
{
    QVariantMap allTypes;

    // Enums
    allTypes.insert("BasicType", basicType());
    allTypes.insert("AuthenticationError", authenticationError());
    allTypes.insert("TunnelProxyError", tunnelProxyError());

    // Types

    return allTypes;
}

void JsonTypes::init()
{
    // Declare types
    s_basicType = enumToStrings(JsonTypes::staticMetaObject, "BasicType");
    s_authenticationError = enumToStrings(Authenticator::staticMetaObject, "AuthenticationError");
    s_tunnelProxyError = enumToStrings(TunnelProxyManager::staticMetaObject, "Error");

    s_initialized = true;
}


QPair<bool, QString> JsonTypes::validateMap(const QVariantMap &templateMap, const QVariantMap &map)
{
    s_lastError.clear();

    // Make sure all values defined in the template are around
    foreach (const QString &key, templateMap.keys()) {
        QString strippedKey = key;
        strippedKey.remove(QRegExp("^o:"));
        if (!key.startsWith("o:") && !map.contains(strippedKey)) {
            qCWarning(dcJsonRpc()) << "*** missing key" << key;
            qCWarning(dcJsonRpc()) << "Expected:      " << templateMap;
            qCWarning(dcJsonRpc()) << "Got:           " << map;
            QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
            return report(false, QString("Missing key %1 in %2").arg(key).arg(QString(jsonDoc.toJson())));
        }
        if (map.contains(strippedKey)) {
            QPair<bool, QString> result = validateVariant(templateMap.value(key), map.value(strippedKey));
            if (!result.first) {
                qCWarning(dcJsonRpc()) << "Object not matching template" << templateMap.value(key) << map.value(strippedKey);
                return result;
            }
        }
    }

    // Make sure there aren't any other parameters than the allowed ones
    foreach (const QString &key, map.keys()) {
        QString optKey = "o:" + key;

        if (!templateMap.contains(key) && !templateMap.contains(optKey)) {
            qCWarning(dcJsonRpc()) << "Forbidden param" << key << "in params";
            QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
            return report(false, QString("Forbidden key \"%1\" in %2").arg(key).arg(QString(jsonDoc.toJson())));
        }
    }

    return report(true, "");
}

QPair<bool, QString> JsonTypes::validateVariant(const QVariant &templateVariant, const QVariant &variant)
{
    switch(templateVariant.type()) {
    case QVariant::String:
        if (templateVariant.toString().startsWith("$ref:")) {
            QString refName = templateVariant.toString();
            if (refName == basicTypeRef()) {
                QPair<bool, QString> result = validateBasicType(variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc()) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(basicTypeRef());
                    return result;
                }
            } else if (refName == authenticationErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_authenticationError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc()) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(authenticationErrorRef());
                    return result;
                }
            } else if (refName == tunnelProxyErrorRef()) {
                QPair<bool, QString> result = validateEnum(s_tunnelProxyError, variant);
                if (!result.first) {
                    qCWarning(dcJsonRpc()) << QString("Value %1 not allowed in %2").arg(variant.toString()).arg(tunnelProxyErrorRef());
                    return result;
                }
            } else {
                Q_ASSERT_X(false, "JsonTypes", QString("Unhandled ref: %1").arg(refName).toLatin1().data());
                return report(false, QString("Unhandled ref %1. Server implementation incomplete.").arg(refName));
            }

        } else {
            QPair<bool, QString> result = JsonTypes::validateProperty(templateVariant, variant);
            if (!result.first) {
                qCWarning(dcJsonRpc()) << "Property not matching:" << templateVariant << "!=" << variant;
                return result;
            }
        }
        break;
    case QVariant::Map: {
        QPair<bool, QString> result = validateMap(templateVariant.toMap(), variant.toMap());
        if (!result.first) {
            return result;
        }
        break;
    }
    case QVariant::List: {
        QPair<bool, QString> result = validateList(templateVariant.toList(), variant.toList());
        if (!result.first) {
            return result;
        }
        break;
    }
    default:
        qCWarning(dcJsonRpc()) << "Unhandled value" << templateVariant;
        return report(false, QString("Unhandled value %1.").arg(templateVariant.toString()));
    }
    return report(true, "");

}

QPair<bool, QString> JsonTypes::validateEnum(const QVariantList &enumList, const QVariant &value)
{
    QStringList enumStrings;
    foreach (const QVariant &variant, enumList) {
        enumStrings.append(variant.toString());
    }

    bool valid = enumStrings.contains(value.toString());
    QString errorMessage = QString("Value %1 not allowed in %2").arg(value.toString()).arg(enumStrings.join(", "));
    if (!valid)
        qCWarning(dcJsonRpc()) << errorMessage;

    return report(valid, errorMessage);
}

QPair<bool, QString> JsonTypes::validateProperty(const QVariant &templateValue, const QVariant &value)
{
    QString strippedTemplateValue = templateValue.toString();

    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Variant)) {
        return report(true, "");
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(JsonTypes::Object)){
        return report(true, "");
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Uuid)) {
        QString errorString = QString("Param %1 is not a uuid.").arg(value.toString());
        return report(value.canConvert(QVariant::Uuid), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::String)) {
        QString errorString = QString("Param %1 is not a string.").arg(value.toString());
        return report(value.canConvert(QVariant::String), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Bool)) {
        QString errorString = QString("Param %1 is not a bool.").arg(value.toString());
        return report(value.canConvert(QVariant::Bool), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::Int)) {
        QString errorString = QString("Param %1 is not a int.").arg(value.toString());
        return report(value.canConvert(QVariant::Int), errorString);
    }
    if (strippedTemplateValue == JsonTypes::basicTypeToString(QVariant::UInt)) {
        QString errorString = QString("Param %1 is not a int.").arg(value.toString());
        return report(value.canConvert(QVariant::UInt), errorString);
    }
    qCWarning(dcJsonRpc()) << QString("Unhandled property type: %1 (expected: %2)").arg(value.toString()).arg(strippedTemplateValue);
    QString errorString = QString("Unhandled property type: %1 (expected: %2)").arg(value.toString()).arg(strippedTemplateValue);
    return report(false, errorString);
}

QPair<bool, QString> JsonTypes::validateList(const QVariantList &templateList, const QVariantList &list)
{
    Q_ASSERT(templateList.count() == 1);
    QVariant entryTemplate = templateList.first();

    for (int i = 0; i < list.count(); ++i) {
        QVariant listEntry = list.at(i);
        QPair<bool, QString> result = validateVariant(entryTemplate, listEntry);
        if (!result.first) {
            qCWarning(dcJsonRpc()) << "List entry not matching template";
            return result;
        }
    }
    return report(true, "");
}

QPair<bool, QString> JsonTypes::validateBasicType(const QVariant &variant)
{
    if (variant.canConvert(QVariant::Uuid) && QVariant(variant).convert(QVariant::Uuid)) {
        if (QUuid(variant.toString()).isNull()) {
            return report(false, "Invalid uuid format.");
        }
        return report(true, "");
    }
    if (variant.canConvert(QVariant::String) && QVariant(variant).convert(QVariant::String)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Int) && QVariant(variant).convert(QVariant::Int)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Double) && QVariant(variant).convert(QVariant::Double)) {
        return report(true, "");
    }
    if (variant.canConvert(QVariant::Bool && QVariant(variant).convert(QVariant::Bool))) {
        return report(true, "");
    }
    return report(false, QString("Error validating basic type %1.").arg(variant.toString()));
}

QString JsonTypes::basicTypeToString(const QVariant::Type &type)
{
    switch (type) {
    case QVariant::Uuid:
        return "Uuid";
        break;
    case QVariant::String:
        return "String";
        break;
    case QVariant::Int:
        return "Int";
        break;
    case QVariant::UInt:
        return "UInt";
        break;
    case QVariant::Double:
        return "Double";
        break;
    case QVariant::Bool:
        return "Bool";
        break;
    default:
        return QVariant::typeToName(static_cast<int>(type));
        break;
    }
}

QPair<bool, QString> JsonTypes::report(bool status, const QString &message)
{
    return qMakePair<bool, QString>(status, message);
}

QVariantList JsonTypes::enumToStrings(const QMetaObject &metaObject, const QString &enumName)
{
    int enumIndex = metaObject.indexOfEnumerator(enumName.toLatin1().data());
    Q_ASSERT_X(enumIndex >= 0, "JsonTypes", QString("Enumerator %1 not found in %2").arg(enumName).arg(metaObject.className()).toLocal8Bit());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    QVariantList enumStrings;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        enumStrings << metaEnum.valueToKey(metaEnum.value(i));
    }
    return enumStrings;
}

}
