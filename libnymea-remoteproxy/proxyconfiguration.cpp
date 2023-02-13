/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

    QSettings settings(m_fileName, QSettings::IniFormat);

    settings.beginGroup("ProxyServer");
    setServerName(settings.value("name", "nymea-remoteproxy").toString());
    setProxyEnabled(settings.value("proxyEnabled", true).toBool());
    setWriteLogFile(settings.value("writeLogs", false).toBool());
    setLogFileName(settings.value("logFile", "/var/log/nymea-remoteproxy.log").toString());
    setLogEngineEnabled(settings.value("logEngineEnabled", false).toBool());
    setMonitorSocketFileName(settings.value("monitorSocket", "/tmp/nymea-remoteproxy.monitor").toString());
    setJsonRpcTimeout(settings.value("jsonRpcTimeout", 10000).toInt());
    setAuthenticationTimeout(settings.value("authenticationTimeout", 8000).toInt());
    setInactiveTimeout(settings.value("inactiveTimeout", 8000).toInt());
    setAloneTimeout(settings.value("aloneTimeout", 8000).toInt());
    settings.endGroup();

    settings.beginGroup("AWS");
    setAwsRegion(settings.value("region", "eu-west-1").toString());
    setAwsAuthorizerLambdaFunctionName(settings.value("authorizerLambdaFunction", "system-services-authorizer-dev-checkToken").toString());
    setAwsCredentialsUrl(QUrl(settings.value("awsCredentialsUrl", "http://169.254.169.254/latest/meta-data/iam/security-credentials/EC2-Remote-Connection-Proxy-Role").toString()));
    settings.endGroup();

    settings.beginGroup("SSL");
    setSslEnabled(settings.value("enabled", true).toBool());
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

    settings.beginGroup("UnixSocketServer");
    setUnixSocketFileName(settings.value("unixSocketFileName", "/run/nymea-remoteproxy.socket").toString());
    settings.endGroup();

    settings.beginGroup("WebSocketServerTunnelProxy");
    setWebSocketServerTunnelProxyHost(QHostAddress(settings.value("host", "127.0.0.1").toString()));
    setWebSocketServerTunnelProxyPort(static_cast<quint16>(settings.value("port", 2212).toInt()));
    settings.endGroup();

    settings.beginGroup("TcpServerTunnelProxy");
    setTcpServerTunnelProxyHost(QHostAddress(settings.value("host", "127.0.0.1").toString()));
    setTcpServerTunnelProxyPort(static_cast<quint16>(settings.value("port", 2213).toInt()));
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

bool ProxyConfiguration::logEngineEnabled() const
{
    return m_logEngineEnabled;
}

void ProxyConfiguration::setLogEngineEnabled(bool enabled)
{
    m_logEngineEnabled = enabled;
}

QString ProxyConfiguration::monitorSocketFileName() const
{
    return m_monitorSocketFileName;
}

void ProxyConfiguration::setMonitorSocketFileName(const QString &fileName)
{
    m_monitorSocketFileName = fileName;
}

int ProxyConfiguration::jsonRpcTimeout() const
{
    return m_jsonRpcTimeout;
}

void ProxyConfiguration::setJsonRpcTimeout(int timeout)
{
    m_jsonRpcTimeout = timeout;
}

int ProxyConfiguration::authenticationTimeout() const
{
    return m_authenticationTimeout;
}

void ProxyConfiguration::setAuthenticationTimeout(int timeout)
{
    m_authenticationTimeout = timeout;
}

int ProxyConfiguration::inactiveTimeout() const
{
    return m_inactiveTimeout;
}

void ProxyConfiguration::setInactiveTimeout(int timeout)
{
    m_inactiveTimeout = timeout;
}

int ProxyConfiguration::aloneTimeout() const
{
    return m_aloneTimeout;
}

void ProxyConfiguration::setAloneTimeout(int timeout)
{
    m_aloneTimeout = timeout;
}

bool ProxyConfiguration::proxyEnabled() const
{
    return m_proxyEnabled;
}

void ProxyConfiguration::setProxyEnabled(bool proxyEnabled)
{
    m_proxyEnabled = proxyEnabled;
}

QString ProxyConfiguration::awsRegion() const
{
    return m_awsRegion;
}

void ProxyConfiguration::setAwsRegion(const QString &region)
{
    m_awsRegion = region;
}

QString ProxyConfiguration::awsAuthorizerLambdaFunctionName() const
{
    return m_awsAuthorizerLambdaFunctionName;
}

void ProxyConfiguration::setAwsAuthorizerLambdaFunctionName(const QString &functionName)
{
    m_awsAuthorizerLambdaFunctionName = functionName;
}

QUrl ProxyConfiguration::awsCredentialsUrl() const
{
    return m_awsCredentialsUrl;
}

void ProxyConfiguration::setAwsCredentialsUrl(const QUrl &url)
{
    m_awsCredentialsUrl = url;
}

bool ProxyConfiguration::sslEnabled() const
{
    return m_sslEnabled;
}

void ProxyConfiguration::setSslEnabled(bool enabled)
{
    m_sslEnabled = enabled;
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

QHostAddress ProxyConfiguration::webSocketServerProxyHost() const
{
    return m_webSocketServerProxyHost;
}

void ProxyConfiguration::setWebSocketServerHost(const QHostAddress &address)
{
    m_webSocketServerProxyHost = address;
}

quint16 ProxyConfiguration::webSocketServerProxyPort() const
{
    return m_webSocketServerProxyPort;
}

void ProxyConfiguration::setWebSocketServerPort(quint16 port)
{
    m_webSocketServerProxyPort = port;
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

QString ProxyConfiguration::unixSocketFileName() const
{
    return m_unixSocketFileName;
}

void ProxyConfiguration::setUnixSocketFileName(const QString &unixSocketFileName)
{
    m_unixSocketFileName = unixSocketFileName;
}

QHostAddress ProxyConfiguration::webSocketServerTunnelProxyHost() const
{
    return m_webSocketServerTunnelProxyHost;
}

void ProxyConfiguration::setWebSocketServerTunnelProxyHost(const QHostAddress &address)
{
    m_webSocketServerTunnelProxyHost = address;
}

quint16 ProxyConfiguration::webSocketServerTunnelProxyPort() const
{
    return m_webSocketServerTunnelProxyPort;
}

void ProxyConfiguration::setWebSocketServerTunnelProxyPort(quint16 port)
{
    m_webSocketServerTunnelProxyPort = port;
}

QHostAddress ProxyConfiguration::tcpServerTunnelProxyHost() const
{
    return m_tcpServerTunnelProxyHost;
}

void ProxyConfiguration::setTcpServerTunnelProxyHost(const QHostAddress &address)
{
    m_tcpServerTunnelProxyHost = address;
}

quint16 ProxyConfiguration::tcpServerTunnelProxyPort() const
{
    return m_tcpServerTunnelProxyPort;
}

void ProxyConfiguration::setTcpServerTunnelProxyPort(quint16 port)
{
    m_tcpServerTunnelProxyPort = port;
}

QDebug operator<<(QDebug debug, ProxyConfiguration *configuration)
{
    debug.nospace() << endl << "========== ProxyConfiguration ==========" << endl;
    debug.nospace() << "Configuration file:" << configuration->fileName() << endl;
    debug.nospace() << "RemoteProxy configuration" << endl;
    debug.nospace() << "  - Server name:" << configuration->serverName() << endl;
    debug.nospace() << "  - Write logfile:" << configuration->writeLogFile() << endl;
    debug.nospace() << "  - Logfile:" << configuration->logFileName() << endl;
    debug.nospace() << "  - Log engine enabled:" << configuration->logEngineEnabled() << endl;
    debug.nospace() << "  - JSON RPC timeout:" << configuration->jsonRpcTimeout() << " [ms]" << endl;
    debug.nospace() << "  - Authentication timeout:" << configuration->authenticationTimeout() << " [ms]" << endl;
    debug.nospace() << "  - Inactive timeout:" << configuration->inactiveTimeout() << " [ms]" << endl;
    debug.nospace() << "  - Alone timeout:" << configuration->aloneTimeout() << " [ms]" << endl;
    debug.nospace() << "SSL configuration" << endl;
    debug.nospace() << "  - Enabled:" << configuration->sslEnabled() << endl;
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
    if (configuration->proxyEnabled()) {
        debug.nospace() << "AWS configuration" << endl;
        debug.nospace() << "  - Region:" << configuration->awsRegion() << endl;
        debug.nospace() << "  - Authorizer lambda function:" << configuration->awsAuthorizerLambdaFunctionName() << endl;
        debug.nospace() << "  - Credentials URL:" << configuration->awsCredentialsUrl().toString() << endl;
        debug.nospace() << "WebSocketServer Proxy" << endl;
        debug.nospace() << "  - Host:" << configuration->webSocketServerProxyHost().toString() << endl;
        debug.nospace() << "  - Port:" << configuration->webSocketServerProxyPort() << endl;
        debug.nospace() << "TcpServer Proxy" << endl;
        debug.nospace() << "  - Host:" << configuration->tcpServerHost().toString() << endl;
        debug.nospace() << "  - Port:" << configuration->tcpServerPort() << endl;
        debug.nospace() << "UnixSocketServer Proxy" << endl;
        debug.nospace() << "  - Filename:" << configuration->unixSocketFileName() << endl;
    } else {
        debug.nospace() << "Proxy Server: disabled" << endl;
    }
    debug.nospace() << "WebSocketServer TunnelProxy" << endl;
    debug.nospace() << "  - Host:" << configuration->webSocketServerTunnelProxyHost().toString() << endl;
    debug.nospace() << "  - Port:" << configuration->webSocketServerTunnelProxyPort() << endl;
    debug.nospace() << "TcpServer TunnelProxy" << endl;
    debug.nospace() << "  - Host:" << configuration->tcpServerTunnelProxyHost().toString() << endl;
    debug.nospace() << "  - Port:" << configuration->tcpServerTunnelProxyPort() << endl;
    debug.nospace() << "========== ProxyConfiguration ==========";
    return debug;
}



}
