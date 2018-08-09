#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QObject>

class JsonRpcClient : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcClient(QObject *parent = nullptr);

signals:

public slots:
};

#endif // JSONRPCCLIENT_H