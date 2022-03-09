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

#include <QUrl>
#include <QHash>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "proxyclient.h"
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
        fprintf(stdout, "%s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtDebugMsg:
        fprintf(stdout, "%s: %s\n", context.category, message.toUtf8().data());
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
    s_loggingFilters.insert("ProxyClient", true);
    s_loggingFilters.insert("RemoteProxyClientJsonRpc", false);
    s_loggingFilters.insert("RemoteProxyClientWebSocket", false);
    s_loggingFilters.insert("RemoteProxyClientConnection", false);
    s_loggingFilters.insert("RemoteProxyClientJsonRpcTraffic", false);
    s_loggingFilters.insert("RemoteProxyClientConnectionTraffic", false);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy client application. This client allowes to test "
                                             "a server application as client perspective.\n\n"
                                             "Version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 2021 nymea GmbH <developer@nymea.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9)));

    QCommandLineOption urlOption(QStringList() << "u" << "url", "The proxy server url. Default ssl://dev-remoteproxy.nymea.io:4433", "url");
    urlOption.setDefaultValue("ssl://dev-remoteproxy.nymea.io:4433");
    parser.addOption(urlOption);

    QCommandLineOption tokenOption(QStringList() << "t" << "token", "The AWS token for authentication.", "token");
    parser.addOption(tokenOption);

    QCommandLineOption nonceOption(QStringList() << "n" << "nonce", "The shared connection unique nonce for this tunnel.", "nonce");
    parser.addOption(nonceOption);

    QCommandLineOption insecureOption(QStringList() << "i" << "ignore-ssl", "Ignore SSL certificate errors.");
    parser.addOption(insecureOption);

    QCommandLineOption pingPongOption(QStringList() << "p" << "pingpong", "Start a ping pong traffic trough the remote connection.");
    parser.addOption(pingPongOption);


    QCommandLineOption nameOption(QStringList() << "name", "The name of the client. Default nymea-remoteproxyclient", "name");
    nameOption.setDefaultValue("nymea-remoteproxyclient");
    parser.addOption(nameOption);

    QCommandLineOption uuidOption(QStringList() << "uuid", "The uuid of the client. If not specified, a new one will be created", "uuid");
    parser.addOption(uuidOption);

    QCommandLineOption wsOption(QStringList() << "ws", "Use web socket as tronsport instead of TCP sockets.");
    parser.addOption(wsOption);

    QCommandLineOption verboseOption(QStringList() << "verbose", "Print more information about the connection.");
    parser.addOption(verboseOption);

    QCommandLineOption veryVerboseOption(QStringList() << "very-verbose", "Print the complete traffic information from the connection.");
    parser.addOption(veryVerboseOption);

    parser.process(application);

    // Enable verbose
    if (parser.isSet(verboseOption) || parser.isSet(veryVerboseOption)) {
        s_loggingFilters["RemoteProxyClientJsonRpc"] = true;
        s_loggingFilters["RemoteProxyClientWebSocket"] = true;
        s_loggingFilters["RemoteProxyClientConnection"] = true;
    }

    // Enable very verbose
    if (parser.isSet(veryVerboseOption)) {
        s_loggingFilters["RemoteProxyClientJsonRpcTraffic"] = true;
        s_loggingFilters["RemoteProxyClientConnectionTraffic"] = true;
    }
    QLoggingCategory::installFilter(loggingCategoryFilter);

    if (!parser.isSet(tokenOption)) {
        qCCritical(dcProxyClient()) << "Please specify the token for authentication using -t <token> or --token <token>." << endl << endl;
        parser.showHelp(-1);
    }

    QUrl serverUrl(parser.value(urlOption));
    if (!serverUrl.isValid()) {
        qCCritical(dcProxyClient()) << "Invalid proxy server url passed." << parser.value(urlOption);
        exit(-1);
    }
    qCDebug(dcProxyClient()) << "Using URL" << serverUrl;

    QUuid uuid(parser.value(uuidOption));
    if (uuid.isNull()) {
        uuid = QUuid::createUuid();
    }

    ProxyClient *client = nullptr;
    if (parser.isSet(wsOption)) {
        qCDebug(dcProxyClient()) << "Using web sockets as transport layer";
        client = new ProxyClient(parser.value(nameOption), uuid, RemoteProxyConnection::ConnectionTypeWebSocket);
        client->setInsecure(parser.isSet(insecureOption));
        client->setPingpong(parser.isSet(pingPongOption));
        client->start(serverUrl, parser.value(tokenOption), parser.value(nonceOption));
    } else {
        qCDebug(dcProxyClient()) << "Using TCP as transport layer";
        client = new ProxyClient(parser.value(nameOption), uuid, RemoteProxyConnection::ConnectionTypeTcpSocket);
        client->setInsecure(parser.isSet(insecureOption));
        client->setPingpong(parser.isSet(pingPongOption));
        client->start(serverUrl, parser.value(tokenOption), parser.value(nonceOption));
    }

    return application.exec();
}
