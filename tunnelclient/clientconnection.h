#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(QObject *parent = nullptr);

signals:

};

#endif // CLIENTCONNECTION_H
