#include "loggingcategories.h"
#include "proxyconfiguration.h"

#include <QFileInfo>

namespace remoteproxy {

ProxyConfiguration::ProxyConfiguration(QObject *parent) :
    QObject(parent)
{

}

bool ProxyConfiguration::loadConfiguration(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        qCWarning(dcApplication()) << "Configuration: Could not find configuration file" << fileName;
        return false;
    }

    if (!fileInfo.isReadable()) {
        qCWarning(dcApplication()) << "Configuration: Cannot read configuration file" << fileName;
        return false;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    qCDebug(dcApplication()) << settings.childGroups();

    setWriteLogFile(settings.value("writeLogs", false).toBool());
    setLogFileName(settings.value("logFile", "/var/log/nymea-remoteproxy.log").toString());
    setSslCertificateFileName(settings.value("certificate", "/etc/ssl/certs/ssl-cert-snakeoil.pem").toString());
    setSslCertificateKeyFileName(settings.value("certificateKey", "/etc/ssl/private/ssl-cert-snakeoil.key").toString());

    settings.beginGroup("WebSocketServer");
    setWebSocketServerHost(QHostAddress(settings.value("host", "127.0.0.1").toString()));
    setWebSocketServerPort(static_cast<quint16>(settings.value("port", 1212).toInt()));
    settings.endGroup();

    settings.beginGroup("TcpServer");
    setWebSocketServerHost(QHostAddress(settings.value("host", "127.0.0.1").toString()));
    setWebSocketServerPort(static_cast<quint16>(settings.value("port", 1213).toInt()));
    settings.endGroup();

    return true;
}

bool ProxyConfiguration::writeLogFile() const
{
    return m_writeLogFile;
}

void ProxyConfiguration::setWriteLogFile(bool enabled)
{
    m_writeLogFile = enabled;
}

QString ProxyConfiguration::logFileName() const
{
    return m_logFileName;
}

void ProxyConfiguration::setLogFileName(const QString &logFileName)
{
    m_logFileName = logFileName;
}

QString ProxyConfiguration::sslCertificateFileName() const
{
    return m_sslCertificateFileName;
}

void ProxyConfiguration::setSslCertificateFileName(const QString &fileName)
{
    m_sslCertificateFileName = fileName;
}

QString ProxyConfiguration::sslCertificateKeyFileName() const
{
    return m_sslCertificateKeyFileName;
}

void ProxyConfiguration::setSslCertificateKeyFileName(const QString &fileName)
{
    m_sslCertificateKeyFileName = fileName;
}

QHostAddress ProxyConfiguration::webSocketServerHost() const
{
    return m_webSocketServerHost;
}

void ProxyConfiguration::setWebSocketServerHost(const QHostAddress &address)
{
    m_webSocketServerHost = address;
}

quint16 ProxyConfiguration::webSocketServerPort() const
{
    return m_webSocketServerPort;
}

void ProxyConfiguration::setWebSocketServerPort(quint16 port)
{
    m_webSocketServerPort = port;
}

QHostAddress ProxyConfiguration::tcpServerHost() const
{
    return m_tcpServerHost;
}

void ProxyConfiguration::setTcpServerHost(const QHostAddress &address)
{
    m_tcpServerHost = address;
}

quint16 ProxyConfiguration::tcpServerPort() const
{
    return m_tcpServerPort;
}

void ProxyConfiguration::setTcpServerPort(quint16 port)
{
    m_tcpServerPort = port;
}

QDebug operator<<(QDebug debug, ProxyConfiguration *configuration)
{
    debug.nospace() << endl << "========== ProxyConfiguration ==========" << endl;
    debug.nospace() << "General" << endl;
    debug.nospace() << "  - write logfile:" << configuration->writeLogFile() << endl;
    debug.nospace() << "  - logfile:" << configuration->logFileName() << endl;
    debug.nospace() << "  - certificate:" << configuration->sslCertificateFileName() << endl;
    debug.nospace() << "  - certificate key:" << configuration->sslCertificateKeyFileName() << endl;
    debug.nospace() << "WebSocketServer" << endl;
    debug.nospace() << "  - host:" << configuration->webSocketServerHost().toString() << endl;
    debug.nospace() << "  - port:" << configuration->webSocketServerPort() << endl;
    debug.nospace() << "TcpServer" << endl;
    debug.nospace() << "  - host:" << configuration->tcpServerHost().toString() << endl;
    debug.nospace() << "  - port:" << configuration->tcpServerPort() << endl;
    debug.nospace() << "========== ProxyConfiguration ==========" << endl;

    return debug;    return debug;
}



}
