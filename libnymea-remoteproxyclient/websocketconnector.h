#ifndef WEBSOCKETCONNECTOR_H
#define WEBSOCKETCONNECTOR_H

#include <QObject>

#include "socketconnector.h"

class WebSocketConnector : public SocketConnector
{
    Q_OBJECT
public:
    explicit WebSocketConnector(QObject *parent = nullptr);

    void sendData(const QByteArray &data) override;

signals:

public slots:
};

#endif // WEBSOCKETCONNECTOR_H
