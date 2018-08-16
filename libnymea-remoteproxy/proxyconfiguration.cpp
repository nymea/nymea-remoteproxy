/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
    m_fileName = fileName;
    QFileInfo fileInfo(m_fileName);
    if (!fileInfo.exists()) {
        qCWarning(dcApplication()) << "Configuration: Could not find configuration file" << m_fileName;
        return false;
    }

    if (!fileInfo.isReadable()) {
        qCWarning(dcApplication()) << "Configuration: Cannot read configuration file" << m_fileName;
        return false;
    }

    QSettings settings(m_fileName, QSettings::IniFormat);

    settings.beginGroup("ProxyServer");
    setServerName(settings.value("name", "nymea-remoteproxy").toString());
    setWriteLogFile(settings.value("writeLogs", false).toBool());
    setLogFileName(settings.value("logFile", "/var/log/nymea-remoteproxy.log").toString());
    setMonitorSocketFileName(settings.value("monitorSocket", "/tmp/nymea-remoteproxy.monitor").toString());
    settings.endGroup();

    settings.beginGroup("SSL");
    setSslCertificateFileName(settings.value("certificate", "/etc/ssl/certs/ssl-cert-snakeoil.pem").toString());
    setSslCertificateKeyFileName(settings.value("certificateKey", "/etc/ssl/private/ssl-cert-snakeoil.key").toString());
    setSslCertificateChainFileName(settings.value("certificateChain", "").toString());
    settings.endGroup();

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
        QSslCertificate certificate(&certChainFile, QSsl::Pem);
        sslConfiguration.setCaCertificates( QList<QSslCertificate>() << certificate );
        certChainFile.close();
        qCDebug(dcApplication()) << "Loaded successfully certificate chain" << sslCertificateKeyFileName();
    }

    m_sslConfiguration = sslConfiguration;

    return true;
}

QString ProxyConfiguration::fileName() const
{
    return m_fileName;
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

QString ProxyConfiguration::monitorSocketFileName() const
{
    return m_monitorSocketFileName;
}

void ProxyConfiguration::setMonitorSocketFileName(const QString &fileName)
{
    m_monitorSocketFileName = fileName;
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
    debug.nospace() << "Configuration file:" << configuration->fileName() << endl;
    debug.nospace() << "RemoteProxy configuration" << endl;
    debug.nospace() << "  - Server name:" << configuration->serverName() << endl;
    debug.nospace() << "  - Write logfile:" << configuration->writeLogFile() << endl;
    debug.nospace() << "  - Logfile:" << configuration->logFileName() << endl;
    debug.nospace() << "SSL configuration" << endl;
    debug.nospace() << "  - Certificate:" << configuration->sslCertificateFileName() << endl;
    debug.nospace() << "  - Certificate key:" << configuration->sslCertificateKeyFileName() << endl;
    debug.nospace() << "  - Certificate chain:" << configuration->sslCertificateChainFileName() << endl;
    debug.nospace() << "  - SSL certificate information:" << endl;
    debug.nospace() << "      Common name:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::CommonName) << endl;
    debug.nospace() << "      Organisation:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::Organization) << endl;
    debug.nospace() << "      Organisation unit name:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::OrganizationalUnitName) << endl;
    debug.nospace() << "      Country name:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::CountryName) << endl;
    debug.nospace() << "      Locality name:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::LocalityName) << endl;
    debug.nospace() << "      State/Province:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::StateOrProvinceName) << endl;
    debug.nospace() << "      Email address:" << configuration->sslConfiguration().localCertificate().subjectInfo(QSslCertificate::EmailAddress) << endl;
    debug.nospace() << "  - SSL certificate issuer information:" << endl;
    debug.nospace() << "      Common name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::CommonName) << endl;
    debug.nospace() << "      Organisation:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::Organization) << endl;
    debug.nospace() << "      Organisation unit name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::OrganizationalUnitName) << endl;
    debug.nospace() << "      Country name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::CountryName) << endl;
    debug.nospace() << "      Locality name:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::LocalityName) << endl;
    debug.nospace() << "      State/Province:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::StateOrProvinceName) << endl;
    debug.nospace() << "      Email address:" << configuration->sslConfiguration().localCertificate().issuerInfo(QSslCertificate::EmailAddress) << endl;
    debug.nospace() << "WebSocketServer configuration" << endl;
    debug.nospace() << "  - Host:" << configuration->webSocketServerHost().toString() << endl;
    debug.nospace() << "  - Port:" << configuration->webSocketServerPort() << endl;
    debug.nospace() << "TcpServer" << endl;
    debug.nospace() << "  - Host:" << configuration->tcpServerHost().toString() << endl;
    debug.nospace() << "  - Port:" << configuration->tcpServerPort() << endl;
    debug.nospace() << "========== ProxyConfiguration ==========";
    return debug;
}



}
