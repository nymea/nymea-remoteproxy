#ifndef JSONTYPES_H
#define JSONTYPES_H

#include <QObject>
#include <QVariant>
#include <QMetaEnum>
#include <QStringList>

#include "authentication/authenticator.h"

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
    Q_ENUMS(BasicType)

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
    DECLARE_TYPE(authenticationError, "AuthenticationError", Authenticator, AuthenticationError)

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

#endif // JSONTYPES_H
