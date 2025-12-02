// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
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

#include <QUrl>
#include <QUuid>
#include <QHash>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "serverconnection.h"
#include "clientconnection.h"

#include "../version.h"

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
        // Enable default debug output
        category->setEnabled(QtDebugMsg, true);
        category->setEnabled(QtWarningMsg, s_loggingFilters.value("qml") || s_loggingFilters.value("Warnings"));
    }
}

static void consoleLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    switch (type) {
    case QtInfoMsg:
        if (context.category == QStringLiteral("default")) {
            fprintf(stdout, "%s\n", message.toUtf8().data());
        } else {
            fprintf(stdout, "%s: %s\n", context.category, message.toUtf8().data());
        }
        break;
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
        fprintf(stdout, "%s%s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    case QtFatalMsg:
        fprintf(stdout, "%s%s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    }
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(consoleLogHandler);

    QCoreApplication application(argc, argv);
    application.setApplicationName(SERVER_NAME_STRING);
    application.setOrganizationName("nymea");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    // Default debug categories
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
    parser.setApplicationDescription(QString("\nThe nymea remote proxy  tunnel client application. This application allowes to register as client"
                                             "or server on the tunnel proxy interface.\n\n"
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

    QCommandLineOption serverOption(QStringList() << "s" << "server", "Connect as tunnel proxy server connection.");
    parser.addOption(serverOption);

    QCommandLineOption clientOption(QStringList() << "c" << "client", "Connect as tunnel proxy client connection. The server uuid is required.");
    parser.addOption(clientOption);

    QCommandLineOption nameOption(QStringList() << "n" << "name", "The name of the connecting client. If not specified a default name will be selected.", "name");
    parser.addOption(nameOption);

    QCommandLineOption randomDataOption(QStringList() << "r" << "random", "Send random data trough the tunnel. If you start a server, it will echo any client data received.");
    parser.addOption(randomDataOption);

    QCommandLineOption uuidOption(QStringList() << "uuid", "The uuid of the connecting client. If not specified, a new one will be created.", "uuid");
    parser.addOption(uuidOption);

    QCommandLineOption serverUuidOption(QStringList() << "server-uuid", "The uuid of the server you want to connect to as client connection.", "uuid");
    parser.addOption(serverUuidOption);

    QCommandLineOption verboseOption(QStringList() << "verbose", "Print more information about the connection.");
    parser.addOption(verboseOption);

    QCommandLineOption veryVerboseOption(QStringList() << "very-verbose", "Print the complete traffic information from the connection.");
    parser.addOption(veryVerboseOption);

    parser.process(application);

    // Make sure not server and client given
    if (parser.isSet(serverOption) && parser.isSet(clientOption)) {
        qCritical() << "Please specify either beeing a client or a server connection, not both.";
        exit(EXIT_FAILURE);
    }

    // Get the uuid to pick for the connection
    QUuid uuid(parser.value(uuidOption));
    if (uuid.isNull()) {
        uuid = QUuid::createUuid();
        qDebug() << "Picking automatic UUID" << uuid.toString();
    }

    QUrl serverUrl(parser.value(urlOption));
    if (!serverUrl.isValid()) {
        qCritical() << "Invalid proxy server url passed." << parser.value(urlOption);
        exit(-1);
    }

    // Server, or if not specified server too
    if (parser.isSet(serverOption) || (!parser.isSet(serverOption) && !parser.isSet(clientOption))) {
        QString name = parser.value("name");
        if (name.isEmpty()) {
            name = "Server connection";
            qDebug() << "Picking automatic name" << name;
        }

        // Enable verbose
        if (parser.isSet(verboseOption) || parser.isSet(veryVerboseOption)) {
            s_loggingFilters["default"] = true;
            s_loggingFilters["TunnelProxySocketServer"] = true;
            s_loggingFilters["TunnelProxyRemoteConnection"] = true;
        }

        // Enable very verbose
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


        // Create the server connection
        ServerConnection *server = new ServerConnection(serverUrl, name, uuid, parser.isSet(insecureOption), parser.isSet(randomDataOption));
        server->startServer();

    } else {
        // Client
        QString name = parser.value("name");
        if (name.isEmpty()) {
            name = "Client connection";
            qDebug() << "Picking automatic name" << name;
        }

        if (!parser.isSet(serverUuidOption)) {
            qCritical() << "Please specify the server UUID you want to connect to using the --server-uuid parameter.";
            exit(EXIT_FAILURE);
        }

        QUuid serverUuid(parser.value(serverUuidOption));
        if (serverUuid.isNull()) {
            qCritical() << "The given server uuid is not valid." << parser.value(serverUuidOption);
            exit(EXIT_FAILURE);
        }

        // Enable verbose
        if (parser.isSet(verboseOption) || parser.isSet(veryVerboseOption)) {
            s_loggingFilters["default"] = true;
            s_loggingFilters["TunnelProxySocketServer"] = true;
            s_loggingFilters["TunnelProxyRemoteConnection"] = true;
        }

        // Enable very verbose
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

        // Create the server connection
        ClientConnection *client = new ClientConnection(serverUrl, name, uuid, serverUuid, parser.isSet(insecureOption), parser.isSet(randomDataOption));
        client->connectToServer();
    }

    return application.exec();
}
