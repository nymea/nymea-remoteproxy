#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QVariantMap>
#include <QMetaMethod>

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

    JsonReply *createReply(const QVariantMap &data) const;
    JsonReply *createAsyncReply(const QString &method) const;

};
#endif // JSONHANDLER_H
