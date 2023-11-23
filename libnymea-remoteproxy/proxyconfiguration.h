/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2023, nymea GmbH
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

#ifndef PROXYCONFIGURATION_H
#define PROXYCONFIGURATION_H

#include <QUrl>
#include <QObject>
#include <QSettings>
#include <QHostAddress>
#include <QSslConfiguration>

namespace remoteproxy {

class ProxyConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConfiguration(QObject *parent = nullptr);

    bool loadConfiguration(const QString &fileName);

    QString fileName() const;

    // ProxyServer
    QString serverName() const;
    void setServerName(const QString &serverName);

    bool writeLogFile() const;
    void setWriteLogFile(bool enabled);

    QString logFileName() const;
    void setLogFileName(const QString &logFileName);

    bool logEngineEnabled() const;
    void setLogEngineEnabled(bool enabled);

    QString monitorSocketFileName() const;
    void setMonitorSocketFileName(const QString &fileName);

    int jsonRpcTimeout() const;
    void setJsonRpcTimeout(int timeout);

    int inactiveTimeout() const;
    void setInactiveTimeout(int timeout);

    // Ssl
    bool sslEnabled() const;
    void setSslEnabled(bool enabled);

    QString sslCertificateFileName() const;
    void setSslCertificateFileName(const QString &fileName);

    QString sslCertificateKeyFileName() const;
    void setSslCertificateKeyFileName(const QString &fileName);

    QString sslCertificateChainFileName() const;
    void setSslCertificateChainFileName(const QString &fileName);

    QSslConfiguration sslConfiguration() const;

    // UnixSocketServer (tunnel)
    QString unixSocketFileName() const;
    void setUnixSocketFileName(const QString &unixSocketFileName);

    // WebSocketServer (tunnel)
    QHostAddress webSocketServerTunnelProxyHost() const;
    void setWebSocketServerTunnelProxyHost(const QHostAddress &address);

    quint16 webSocketServerTunnelProxyPort() const;
    void setWebSocketServerTunnelProxyPort(quint16 port);

    // TcpServer (tunnel)
    QHostAddress tcpServerTunnelProxyHost() const;
    void setTcpServerTunnelProxyHost(const QHostAddress &address);

    quint16 tcpServerTunnelProxyPort() const;
    void setTcpServerTunnelProxyPort(quint16 port);

private:
    // ProxyServer
    QString m_fileName;
    QString m_serverName;
    bool m_writeLogFile = false;
    QString m_logFileName = "/var/log/nymea-remoteproxy.log";
    bool m_logEngineEnabled = false;
    QString m_monitorSocketFileName;

    int m_jsonRpcTimeout = 10000;
    int m_inactiveTimeout = 8000;

    // Ssl
    bool m_sslEnabled = true;
    QString m_sslCertificateFileName = "/etc/ssl/certs/ssl-cert-snakeoil.pem";
    QString m_sslCertificateKeyFileName = "/etc/ssl/private/ssl-cert-snakeoil.key";
    QString m_sslCertificateChainFileName;
    QSslConfiguration m_sslConfiguration;

    // UnixSocketServer (tunnel)
    QString m_unixSocketFileName = "/run/nymea-remoteproxy.socket";

    // WebSocketServer (tunnel)
    QHostAddress m_webSocketServerTunnelProxyHost = QHostAddress::LocalHost;
    quint16 m_webSocketServerTunnelProxyPort = 2212;

    // TcpServer (tunnel)
    QHostAddress m_tcpServerTunnelProxyHost = QHostAddress::LocalHost;
    quint16 m_tcpServerTunnelProxyPort = 2213;

};

QDebug operator<< (QDebug debug, ProxyConfiguration *configuration);

}

#endif // PROXYCONFIGURATION_H
