// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <cstdlib>
#include <ctime>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDateTime>
#include <QHash>
#include <QLoggingCategory>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include "tunnelproxy/tunnelproxyremoteconnection.h"
#include "tunnelproxy/tunnelproxysocket.h"
#include "tunnelproxy/tunnelproxysocketserver.h"

#include "../version.h"

using namespace remoteproxyclient;

static QHash<QString, bool> s_loggingFilters;
static const char *const normal = "\033[0m";
static const char *const warning = "\e[33m";
static const char *const error = "\e[31m";

static void loggingCategoryFilter(QLoggingCategory *category)
{
    if (s_loggingFilters.contains(category->categoryName())) {
        bool debugEnabled = s_loggingFilters.value(category->categoryName());
        category->setEnabled(QtDebugMsg, debugEnabled);
        category->setEnabled(QtWarningMsg, debugEnabled || s_loggingFilters.value("Warnings"));
    } else {
        category->setEnabled(QtDebugMsg, s_loggingFilters.value("default"));
        category->setEnabled(QtWarningMsg, s_loggingFilters.value("Warnings"));
    }
}

static void consoleLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    switch (type) {
    case QtInfoMsg:
    case QtDebugMsg:
        if (context.category == QStringLiteral("default")) {
            fprintf(stdout, "%s\n", message.toUtf8().data());
        } else {
            fprintf(stdout, "%s: %s\n", context.category, message.toUtf8().data());
        }
        break;
    case QtWarningMsg:
        fprintf(stdout, "%s%s: %s%s\n", warning, context.category, message.toUtf8().data(), normal);
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        fprintf(stdout, "%s%s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    }
    fflush(stdout);
}

class TunnelProxyTest : public QObject
{
    Q_OBJECT
public:
    explicit TunnelProxyTest(const QUrl &serverUrl, const QString &serverName, const QString &clientName, const QUuid &serverUuid, const QUuid &clientUuid, bool insecure, int timeout, QObject *parent = nullptr) :
        QObject(parent),
        m_serverUrl(serverUrl),
        m_serverName(serverName),
        m_clientName(clientName),
        m_serverUuid(serverUuid),
        m_clientUuid(clientUuid),
        m_insecure(insecure)
    {
        m_timeoutTimer.setInterval(timeout);
        m_timeoutTimer.setSingleShot(true);
        connect(&m_timeoutTimer, &QTimer::timeout, this, [this](){
            fail("Timed out while waiting for tunnel proxy test to finish.");
        });
    }

    void start()
    {
        qInfo() << "Starting tunnel proxy test against" << m_serverUrl.toString();
        qInfo() << "Server UUID:" << m_serverUuid.toString();
        qInfo() << "Client UUID:" << m_clientUuid.toString();

        m_clientToServerPayload = randomPayload(128);
        m_serverToClientPayload = randomPayload(128);
        while (m_clientToServerPayload == m_serverToClientPayload) {
            m_serverToClientPayload = randomPayload(128);
        }

        m_timeoutTimer.start();

        m_server = new TunnelProxySocketServer(m_serverUuid, m_serverName, this);
        connect(m_server, &TunnelProxySocketServer::runningChanged, this, &TunnelProxyTest::onServerRunningChanged);
        connect(m_server, &TunnelProxySocketServer::clientConnected, this, &TunnelProxyTest::onServerClientConnected);
        connect(m_server, &TunnelProxySocketServer::clientDisconnected, this, &TunnelProxyTest::onServerClientDisconnected);
        connect(m_server, &TunnelProxySocketServer::errorOccurred, this, [this](QAbstractSocket::SocketError socketError){
            fail(QString("Server socket error occurred: %1").arg(socketError));
        });
        connect(m_server, &TunnelProxySocketServer::serverErrorOccurred, this, [this](TunnelProxySocketServer::Error serverError){
            fail(QString("Tunnel proxy server error occurred: %1").arg(serverError));
        });
        connect(m_server, &TunnelProxySocketServer::sslErrors, this, [this](const QList<QSslError> &errors){
            if (m_insecure) {
                m_server->ignoreSslErrors(errors);
                return;
            }

            QStringList errorStrings;
            foreach (const QSslError &sslError, errors) {
                errorStrings.append(sslError.errorString());
            }
            fail(QString("Server SSL errors occurred: %1").arg(errorStrings.join(", ")));
        });

        if (!m_server->startServer(m_serverUrl)) {
            fail("Failed to start tunnel proxy server connection.");
        }
    }

private slots:
    void onServerRunningChanged(bool running)
    {
        if (m_finished)
            return;

        if (!running) {
            fail("Server connection stopped before the test finished.");
            return;
        }

        qInfo() << "Server registered. Connecting client shortly.";
        QTimer::singleShot(500, this, &TunnelProxyTest::startClient);
    }

    void startClient()
    {
        if (m_finished)
            return;

        m_client = new TunnelProxyRemoteConnection(m_clientUuid, m_clientName, this);
        connect(m_client, &TunnelProxyRemoteConnection::remoteConnectedChanged, this, &TunnelProxyTest::onClientRemoteConnectedChanged);
        connect(m_client, &TunnelProxyRemoteConnection::dataReady, this, &TunnelProxyTest::onClientDataReady);
        connect(m_client, &TunnelProxyRemoteConnection::errorOccurred, this, [this](QAbstractSocket::SocketError socketError){
            fail(QString("Client socket error occurred: %1").arg(socketError));
        });
        connect(m_client, &TunnelProxyRemoteConnection::sslErrors, this, [this](const QList<QSslError> &errors){
            if (m_insecure) {
                m_client->ignoreSslErrors(errors);
                return;
            }

            QStringList errorStrings;
            foreach (const QSslError &sslError, errors) {
                errorStrings.append(sslError.errorString());
            }
            fail(QString("Client SSL errors occurred: %1").arg(errorStrings.join(", ")));
        });

        if (!m_client->connectServer(m_serverUrl, m_serverUuid)) {
            fail("Failed to start tunnel proxy client connection.");
        }
    }

    void onServerClientConnected(TunnelProxySocket *socket)
    {
        if (m_finished)
            return;

        qInfo() << "Client connected to server tunnel.";
        m_tunnelSocket = socket;
        connect(m_tunnelSocket, &TunnelProxySocket::dataReceived, this, &TunnelProxyTest::onServerDataReceived);
        connect(m_tunnelSocket, &TunnelProxySocket::disconnected, this, [this](){
            if (!m_finished) {
                fail("Tunnel socket disconnected before the test finished.");
            }
        });
        maybeSendClientPayload();
    }

    void onServerClientDisconnected(TunnelProxySocket *socket)
    {
        if (socket == m_tunnelSocket) {
            m_tunnelSocket = nullptr;
        }
    }

    void onClientRemoteConnectedChanged(bool remoteConnected)
    {
        if (m_finished)
            return;

        if (!remoteConnected) {
            fail("Client remote connection closed before the test finished.");
            return;
        }

        qInfo() << "Client remote connection established.";
        m_clientRemoteConnected = true;
        maybeSendClientPayload();
    }

    void onServerDataReceived(const QByteArray &data)
    {
        if (m_finished)
            return;

        if (data != m_clientToServerPayload) {
            fail("Server received unexpected client payload.");
            return;
        }

        qInfo() << "Client-to-server payload verified.";
        m_serverReceivedClientPayload = true;

        if (!m_tunnelSocket || !m_tunnelSocket->connected()) {
            fail("Cannot send server payload. Tunnel socket is not connected.");
            return;
        }

        qInfo() << "Sending server-to-client payload.";
        m_tunnelSocket->writeData(m_serverToClientPayload);
    }

    void onClientDataReady(const QByteArray &data)
    {
        if (m_finished)
            return;

        if (!m_serverReceivedClientPayload) {
            fail("Client received data before the server payload was sent.");
            return;
        }

        if (data != m_serverToClientPayload) {
            fail("Client received unexpected server payload.");
            return;
        }

        qInfo() << "Server-to-client payload verified.";
        succeed();
    }

private:
    QUrl m_serverUrl;
    QString m_serverName;
    QString m_clientName;
    QUuid m_serverUuid;
    QUuid m_clientUuid;
    bool m_insecure = false;
    bool m_finished = false;
    bool m_clientRemoteConnected = false;
    bool m_clientPayloadSent = false;
    bool m_serverReceivedClientPayload = false;

    TunnelProxySocketServer *m_server = nullptr;
    TunnelProxyRemoteConnection *m_client = nullptr;
    TunnelProxySocket *m_tunnelSocket = nullptr;

    QByteArray m_clientToServerPayload;
    QByteArray m_serverToClientPayload;
    QTimer m_timeoutTimer;

    void maybeSendClientPayload()
    {
        if (m_clientPayloadSent || !m_clientRemoteConnected || !m_tunnelSocket)
            return;

        qInfo() << "Sending client-to-server payload.";
        if (!m_client->sendData(m_clientToServerPayload)) {
            fail("Failed to send client payload.");
            return;
        }
        m_clientPayloadSent = true;
    }

    void succeed()
    {
        if (m_finished)
            return;

        m_finished = true;
        m_timeoutTimer.stop();
        qInfo() << "Tunnel proxy test finished successfully.";
        disconnectConnections();
        QTimer::singleShot(250, qApp, [](){
            QCoreApplication::exit(EXIT_SUCCESS);
        });
    }

    void fail(const QString &message)
    {
        if (m_finished)
            return;

        m_finished = true;
        m_timeoutTimer.stop();
        qCritical() << message;
        disconnectConnections();
        QTimer::singleShot(250, qApp, [](){
            QCoreApplication::exit(EXIT_FAILURE);
        });
    }

    void disconnectConnections()
    {
        if (m_tunnelSocket && m_tunnelSocket->connected()) {
            m_tunnelSocket->disconnectSocket();
        }

        if (m_client) {
            m_client->disconnectServer();
        }

        if (m_server) {
            m_server->stopServer();
        }
    }

    QByteArray randomPayload(int length) const
    {
        static const char possibleCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray payload;
        payload.reserve(length);
        for (int i = 0; i < length; ++i) {
            payload.append(possibleCharacters[std::rand() % (sizeof(possibleCharacters) - 1)]);
        }
        return payload;
    }
};

int main(int argc, char *argv[])
{
    qInstallMessageHandler(consoleLogHandler);

    QCoreApplication application(argc, argv);
    application.setApplicationName("nymea-tunnelproxy-testutils");
    application.setOrganizationName("nymea");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    std::srand(static_cast<unsigned int>(QDateTime::currentMSecsSinceEpoch()));

    s_loggingFilters.insert("default", false);
    s_loggingFilters.insert("Warnings", true);
    s_loggingFilters.insert("RemoteProxyClientJsonRpc", false);
    s_loggingFilters.insert("RemoteProxyClientJsonRpcTraffic", false);
    s_loggingFilters.insert("RemoteProxyClientWebSocket", false);
    s_loggingFilters.insert("RemoteProxyClientTcpSocket", false);
    s_loggingFilters.insert("RemoteProxyClientConnection", false);
    s_loggingFilters.insert("RemoteProxyClientConnectionTraffic", false);
    s_loggingFilters.insert("TunnelProxySocketServer", false);
    s_loggingFilters.insert("TunnelProxySocketServerTraffic", false);
    s_loggingFilters.insert("TunnelProxyRemoteConnection", false);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea tunnel proxy online test utility.\n\n"
                                             "Version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 %4 nymea GmbH <developer@nymea.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9))
                                     .arg(COPYRIGHT_YEAR_STRING));

    QCommandLineOption urlOption(QStringList() << "u" << "url", "The proxy server url. Default ssl://dev-remoteproxy.nymea.io:2213", "url");
    urlOption.setDefaultValue("ssl://dev-remoteproxy.nymea.io:2213");
    parser.addOption(urlOption);

    QCommandLineOption insecureOption(QStringList() << "i" << "ignore-ssl", "Ignore SSL certificate errors.");
    parser.addOption(insecureOption);

    QCommandLineOption nameOption(QStringList() << "n" << "name", "The base name of the test connections. If not specified a default name will be selected.", "name");
    parser.addOption(nameOption);

    QCommandLineOption uuidOption(QStringList() << "uuid", "The UUID of the tunnel server connection. If not specified, a new one will be created.", "uuid");
    parser.addOption(uuidOption);

    QCommandLineOption clientUuidOption(QStringList() << "client-uuid", "The UUID of the tunnel client connection. If not specified, a new one will be created.", "uuid");
    parser.addOption(clientUuidOption);

    QCommandLineOption timeoutOption(QStringList() << "timeout", "Timeout for the complete test in milliseconds. Default: 30000", "ms");
    timeoutOption.setDefaultValue("30000");
    parser.addOption(timeoutOption);

    QCommandLineOption verboseOption(QStringList() << "verbose", "Print more information about the connection.");
    parser.addOption(verboseOption);

    QCommandLineOption veryVerboseOption(QStringList() << "very-verbose", "Print the complete traffic information from the connection.");
    parser.addOption(veryVerboseOption);

    parser.process(application);

    QUrl serverUrl(parser.value(urlOption));
    if (!serverUrl.isValid() || serverUrl.isEmpty()) {
        qCritical() << "Invalid proxy server url passed." << parser.value(urlOption);
        return EXIT_FAILURE;
    }

    bool timeoutOk = false;
    int timeout = parser.value(timeoutOption).toInt(&timeoutOk);
    if (!timeoutOk || timeout <= 0) {
        qCritical() << "Invalid timeout passed." << parser.value(timeoutOption);
        return EXIT_FAILURE;
    }

    QUuid serverUuid(parser.value(uuidOption));
    if (serverUuid.isNull()) {
        serverUuid = QUuid::createUuid();
    }

    QUuid clientUuid(parser.value(clientUuidOption));
    if (clientUuid.isNull()) {
        clientUuid = QUuid::createUuid();
    }

    QString name = parser.value(nameOption);
    if (name.isEmpty()) {
        name = "Tunnel proxy test";
    }

    if (parser.isSet(verboseOption) || parser.isSet(veryVerboseOption)) {
        s_loggingFilters["default"] = true;
        s_loggingFilters["TunnelProxySocketServer"] = true;
        s_loggingFilters["TunnelProxyRemoteConnection"] = true;
    }

    if (parser.isSet(veryVerboseOption)) {
        s_loggingFilters["RemoteProxyClientJsonRpc"] = true;
        s_loggingFilters["RemoteProxyClientJsonRpcTraffic"] = true;
        s_loggingFilters["RemoteProxyClientWebSocket"] = true;
        s_loggingFilters["RemoteProxyClientTcpSocket"] = true;
        s_loggingFilters["RemoteProxyClientConnection"] = true;
        s_loggingFilters["RemoteProxyClientConnectionTraffic"] = true;
        s_loggingFilters["TunnelProxySocketServerTraffic"] = true;
    }

    QLoggingCategory::installFilter(loggingCategoryFilter);

    TunnelProxyTest test(serverUrl, name + " server", name + " client", serverUuid, clientUuid, parser.isSet(insecureOption), timeout);
    QTimer::singleShot(0, &test, &TunnelProxyTest::start);

    return application.exec();
}

#include "main.moc"
