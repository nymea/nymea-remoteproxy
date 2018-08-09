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

    static JsonReply *createReply(JsonHandler *handler, const QVariantMap &data);
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
