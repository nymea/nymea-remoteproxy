#ifndef WEBSOCKETCONNECTOR_H
#define WEBSOCKETCONNECTOR_H

#include <QObject>

class WebSocketConnector : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketConnector(QObject *parent = nullptr);

signals:

public slots:
};

#endif // WEBSOCKETCONNECTOR_H