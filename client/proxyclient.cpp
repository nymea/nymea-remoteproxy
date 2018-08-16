#include "proxyclient.h"

Q_LOGGING_CATEGORY(dcProxyClient, "ProxyClient")

ProxyClient::ProxyClient(const QString &name, const QUuid &uuid, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_uuid(uuid)
{
    m_connection = new RemoteProxyConnection(m_uuid, m_name, this);
    qCDebug(dcProxyClient()) << "Creating remote proxy connection" << m_name << m_uuid.toString();
    connect(m_connection, &RemoteProxyConnection::ready, this, &ProxyClient::onClientReady);
    connect(m_connection, &RemoteProxyConnection::authenticated, this, &ProxyClient::onAuthenticationFinished);
    connect(m_connection, &RemoteProxyConnection::remoteConnectionEstablished, this, &ProxyClient::onRemoteConnectionEstablished);
    connect(m_connection, &RemoteProxyConnection::errorOccured, this, &ProxyClient::onErrorOccured);
    connect(m_connection, &RemoteProxyConnection::disconnected, this, &ProxyClient::onClientDisconnected);
    connect(m_connection, &RemoteProxyConnection::sslErrors, this, &ProxyClient::onSslErrors);
}

void ProxyClient::setInsecure(bool insecure)
{
    m_insecure = insecure;
}

void ProxyClient::onErrorOccured(RemoteProxyConnection::Error error)
{
    qCWarning(dcProxyClient()) << "Error occured" << error << m_connection->errorString();
    exit(-1);
}

void ProxyClient::onClientReady()
{
    qCDebug(dcProxyClient()) << "Connected to proxy server" << m_connection->serverUrl().toString();
    qCDebug(dcProxyClient()) << "Start authentication";
    m_connection->authenticate(m_token);
}

void ProxyClient::onAuthenticationFinished()
{
    qCDebug(dcProxyClient()) << "Authentication finished successfully.";
}

void ProxyClient::onRemoteConnectionEstablished()
{
    qCDebug(dcProxyClient()) << "----------------------------------------------------------------------------------";
    qCDebug(dcProxyClient()) << "Remote connection established with" << m_connection->tunnelPartnerName() << m_connection->tunnelPartnerUuid();
    qCDebug(dcProxyClient()) << "----------------------------------------------------------------------------------";
}

void ProxyClient::onClientDisconnected()
{
    qCDebug(dcProxyClient()) << "Disconnected from" << m_connection->serverUrl().toString();
    exit(1);
}

void ProxyClient::onSslErrors(const QList<QSslError> errors)
{
    if (m_insecure) {
        qCDebug(dcProxyClient()) << "SSL errors occured. Ignoring because explicit specified.";
        m_connection->ignoreSslErrors();
    } else {
        qCWarning(dcProxyClient()) << "SSL errors occured:";
        foreach (const QSslError &sslError, errors) {
            qCWarning(dcProxyClient()) << "  --> " << sslError.errorString();
        }
    }
}

void ProxyClient::start(const QUrl &url, const QString &token)
{
    m_token = token;
    m_connection->connectServer(url);
}
