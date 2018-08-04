#include "transportinterface.h"

TransportInterface::TransportInterface(QObject *parent) :
    QObject(parent)
{

}

QString TransportInterface::serverName() const
{
    return m_serverName;
}

TransportInterface::~TransportInterface()
{

}
