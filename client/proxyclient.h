#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QObject>


class ProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit ProxyClient(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PROXYCLIENT_H
