#ifndef PROXYCONFIGURATION_H
#define PROXYCONFIGURATION_H

#include <QObject>
#include <QSettings>
#include <QHostAddress>

namespace remoteproxy {

class ProxyConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConfiguration(QObject *parent = nullptr);

    bool loadConfiguration(const QString &fileName);

    // General
    bool writeLogFile() const;
    void setWriteLogFile(bool enabled);

    QString logFileName() const;
    void setLogFileName(const QString &logFileName);

    QString sslCertificateFileName() const;
    void setSslCertificateFileName(const QString &fileName);

    QString sslCertificateKeyFileName() const;
    void setSslCertificateKeyFileName(const QString &fileName);

    // WebSocketServer
    QHostAddress webSocketServerHost() const;
    void setWebSocketServerHost(const QHostAddress &address);

    quint16 webSocketServerPort() const;
    void setWebSocketServerPort(quint16 port);

    // TcpServer
    QHostAddress tcpServerHost() const;
    void setTcpServerHost(const QHostAddress &address);

    quint16 tcpServerPort() const;
    void setTcpServerPort(quint16 port);

private:
    // General
    bool m_writeLogFile = false;
    QString m_logFileName = "/var/log/nymea-remoteproxy.log";
    QString m_sslCertificateFileName = "/etc/ssl/certs/ssl-cert-snakeoil.pem";
    QString m_sslCertificateKeyFileName = "/etc/ssl/private/ssl-cert-snakeoil.key";

    // WebSocketServer
    QHostAddress m_webSocketServerHost = QHostAddress::LocalHost;
    quint16 m_webSocketServerPort = 1212;

    // TcpServer
    QHostAddress m_tcpServerHost = QHostAddress::LocalHost;
    quint16 m_tcpServerPort = 1213;

};

QDebug operator<< (QDebug debug, ProxyConfiguration *configuration);

}

#endif // PROXYCONFIGURATION_H
