#ifndef PROXYCONFIGURATION_H
#define PROXYCONFIGURATION_H

#include <QObject>

class ProxyConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConfiguration(QObject *parent = nullptr);

signals:

public slots:

};

#endif // PROXYCONFIGURATION_H