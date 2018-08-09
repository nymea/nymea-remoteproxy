#include "websocketconnector.h"

WebSocketConnector::WebSocketConnector(QObject *parent) :
    SocketConnector(parent)
{

}

void WebSocketConnector::sendData(const QByteArray &data)
{
    Q_UNUSED(data)
}
