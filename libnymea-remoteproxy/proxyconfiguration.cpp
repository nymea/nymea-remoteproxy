#include "loggingcategories.h"
#include "proxyconfiguration.h"

#include <QFile>
#include <QSslKey>
#include <QFileInfo>
#include <QSslCertificate>

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

    setServerName(settings.value("name", "nymea-remoteproxy").toString());
    setWriteLogFile(settings.value("writeLogs", false).toBool());
    setLogFileName(settings.value("logFile", "/var/log/nymea-remoteproxy.log").toString());
    setSslCertificateFileName(settings.value("certificate", "/etc/ssl/certs/ssl-cert-snakeoil.pem").toString());
    setSslCertificateKeyFileName(settings.value("certificateKey", "/etc/ssl/private/ssl-cert-snakeoil.key").toString());
    setSslCertificateChainFileName(settings.value("certificateChain", "").toString());

    settings.beginGroup("WebSocketServer");
    setWebSocketServerHost(QHostAddress(settings.value("host", "127.0.0.1").toString()));
    setWebSocketServerPort(static_cast<quint16>(settings.value("port", 1212).toInt()));
    settings.endGroup();

    settings.beginGroup("TcpServer");
    setTcpServerHost(QHostAddress(settings.value("host", "127.0.0.1").toString()));
    setTcpServerPort(static_cast<quint16>(settings.value("port", 1213).toInt()));
    settings.endGroup();

    // Load SSL configuration
    QSslConfiguration sslConfiguration;
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);

    // SSL certificate
    QFile certFile(sslCertificateFileName());
    if (!certFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcApplication()) << "Could not open certificate file" << sslCertificateFileName() << certFile.errorString();
        return false;
    }
    QSslCertificate certificate(&certFile, QSsl::Pem);
    qCDebug(dcApplication()) << "Loaded successfully certificate" << sslCertificateFileName();
    certFile.close();
    sslConfiguration.setLocalCertificate(certificate);

    // SSL certificate key
    QFile certKeyFile(sslCertificateKeyFileName());
    if (!certKeyFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcApplication()) << "Could not open certificate key file:" << sslCertificateKeyFileName() << certKeyFile.errorString();
        return false;
    }
    QSslKey sslKey(&certKeyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    qCDebug(dcApplication()) << "Loaded successfully certificate key" << sslCertificateKeyFileName();
    certKeyFile.close();
    sslConfiguration.setPrivateKey(sslKey);

    // SSL certificate chain
    if (!sslCertificateChainFileName().isEmpty()) {
        QFile certChainFile(sslCertificateChainFileName());
        if (!certChainFile.open(QIODevice::ReadOnly)) {
            qCWarning(dcApplication()) << "Could not open certificate chain file:" << sslCertificateChainFileName() << certChainFile.errorString();
            return false;
        }
        QSslCertificate certificate(&certKeyFile, QSsl::Pem);
        sslConfiguration.setLocalCertificateChain( { certificate } );
    }

    m_sslConfiguration = sslConfiguration;

    return true;
}

QString ProxyConfiguration::serverName() const
{
    return m_serverName;
}

void ProxyConfiguration::setServerName(const QString &serverName)
{
    m_serverName = serverName;
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

QString ProxyConfiguration::sslCertificateChainFileName() const
{
    return m_sslCertificateChainFileName;
}

void ProxyConfiguration::setSslCertificateChainFileName(const QString &fileName)
{
    m_sslCertificateChainFileName = fileName;
}

QSslConfiguration ProxyConfiguration::sslConfiguration() const
{
    return m_sslConfiguration;
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
    debug.nospace() << "  - name:" << configuration->serverName() << endl;
    debug.nospace() << "  - write logfile:" << configuration->writeLogFile() << endl;
    debug.nospace() << "  - logfile:" << configuration->logFileName() << endl;
    debug.nospace() << "  - certificate:" << configuration->sslCertificateFileName() << endl;
    debug.nospace() << "  - certificate key:" << configuration->sslCertificateKeyFileName() << endl;
    debug.nospace() << "  - certificate chain:" << configuration->sslCertificateChainFileName() << endl;
    debug.nospace() << "  - SSL certificate information:";
    debug.nospace() << "      Common name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::CommonName) << endl;
    debug.nospace() << "      Organisation:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::Organization) << endl;
    debug.nospace() << "      Organisation unit name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::OrganizationalUnitName) << endl;
    debug.nospace() << "      Country name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::CountryName) << endl;
    debug.nospace() << "      Locality name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::LocalityName) << endl;
    debug.nospace() << "      State/Province:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::StateOrProvinceName) << endl;
    debug.nospace() << "      Email address:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::EmailAddress) << endl;
    debug.nospace() << "WebSocketServer" << endl;
    debug.nospace() << "  - host:" << configuration->webSocketServerHost().toString() << endl;
    debug.nospace() << "  - port:" << configuration->webSocketServerPort() << endl;
    debug.nospace() << "TcpServer" << endl;
    debug.nospace() << "  - host:" << configuration->tcpServerHost().toString() << endl;
    debug.nospace() << "  - port:" << configuration->tcpServerPort() << endl;
    debug.nospace() << "========== ProxyConfiguration ==========" << endl;

    return debug;
}



}
