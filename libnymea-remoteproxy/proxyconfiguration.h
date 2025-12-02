// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
