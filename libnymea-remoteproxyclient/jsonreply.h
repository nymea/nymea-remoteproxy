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
