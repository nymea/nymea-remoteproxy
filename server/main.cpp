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

#include <QDir>
#include <QUrl>
#include <QtDebug>
#include <QSslKey>
#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageLogger>
#include <QSslCertificate>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QSslConfiguration>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>

#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "loggingcategories.h"
#include "proxyconfiguration.h"
#include "remoteproxyserverapplication.h"
#include "authentication/aws/awsauthenticator.h"
#include "authentication/dummy/dummyauthenticator.h"

#include "../version.h"

using namespace remoteproxy;

static QHash<QString, bool> s_loggingFilters;

static QFile s_logFile;
static bool s_loggingEnabled = false;

static const char *const normal = "\033[0m";
static const char *const warning = "\e[33m";
static const char *const error = "\e[31m";

static void consoleLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QString messageString;
    QString timeString = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
    switch (type) {
    case QtInfoMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtDebugMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtWarningMsg:
        messageString = QString(" W %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s W | %s: %s%s\n", warning, context.category, message.toUtf8().data(), normal);
        break;
    case QtCriticalMsg:
        messageString = QString(" C %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s C | %s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    case QtFatalMsg:
        messageString = QString(" F %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s F | %s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    }
    fflush(stdout);

    if (s_logFile.isOpen()) {
        QTextStream textStream(&s_logFile);
        textStream << messageString << endl;
    }
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(consoleLogHandler);

    RemoteProxyServerApplication application(argc, argv);
    application.setApplicationName(SERVER_NAME_STRING);
    application.setOrganizationName("nymea");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    QString configFile = "/etc/nymea/nymea-remoteproxy.conf";

    // command line parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy server. This server allowes nymea-cloud users and "
                                             "registered nymea deamons to establish a tunnel connection.\n\n"
                                             "Version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 %4 nymea GmbH <developer@nymea.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9))
                                     .arg(COPYRIGHT_YEAR_STRING));

    QCommandLineOption logfileOption(QStringList() << "l" << "logging", "Write log file to the given logfile.",
                                     "logfile", "/var/log/nymea-remoteproxy.log");
    parser.addOption(logfileOption);

    QCommandLineOption developmentOption(QStringList() << "d" << "development", "Enable the development mode. This enabled the server "
                                                                                "assumes there are static AWS credentials provided to aws-cli.");
    parser.addOption(developmentOption);

    QCommandLineOption mockAuthenticatorOption(QStringList() << "m" << "mock-authenticator", "Start the server using a mock authenticator which returns always true.");
    parser.addOption(mockAuthenticatorOption);

    QCommandLineOption configOption(QStringList() << "c" <<"configuration", "The path to the proxy server configuration file. The default is " + configFile, "configuration");
    configOption.setDefaultValue(configFile);
    parser.addOption(configOption);

    QCommandLineOption verboseOption(QStringList() << "verbose", "Print more verbose.");
    parser.addOption(verboseOption);

    parser.process(application);

    // Create a default configuration
    ProxyConfiguration *configuration = new ProxyConfiguration(nullptr);
    if (parser.isSet(configOption))
        configFile = parser.value(configOption);

    qCDebug(dcApplication()) << "Loading configuration file from" << configFile;
    if (!configuration->loadConfiguration(parser.value(configOption))) {
        qCCritical(dcApplication()) << "Invalid configuration file passed" << parser.value(configOption);
        exit(-1);
    }

    if (parser.isSet(logfileOption)) {
        configuration->setWriteLogFile(true);
        configuration->setLogFileName(parser.value(logfileOption));
    }

    // Open logfile if configured
    if (configuration->writeLogFile()) {
        s_loggingEnabled = true;
        QFileInfo fi(configuration->logFileName());
        QDir dir(fi.absolutePath());
        if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
            qCWarning(dcApplication()) << "Error opening log file" << configuration->logFileName();
            exit(-1);
        }
        s_logFile.setFileName(configuration->logFileName());
        if (!s_logFile.open(QFile::WriteOnly | QFile::Append)) {
            qWarning() << "Error opening log file" << configuration->logFileName();
            exit(-1);
        }
    }

    // Verify webserver configuration
    if (configuration->webSocketServerProxyHost().isNull()) {
        qCCritical(dcApplication()) << "Invalid web socket host address passed.";
        exit(-1);
    }

    // Verify tcp server configuration
    if (configuration->tcpServerHost().isNull()) {
        qCCritical(dcApplication()) << "Invalid TCP server host address passed.";
        exit(-1);
    }

    // Verify SSL configuration
    if (configuration->sslEnabled() && configuration->sslConfiguration().isNull()) {
        qCCritical(dcApplication()) << "SSL is enabled but no SSL configuration specified.";
        exit(-1);
    } else {
        qCDebug(dcApplication()) << "Using SSL version:" << QSslSocket::sslLibraryVersionString();
    }

    qCDebug(dcApplication()) << "==========================================================";
    qCDebug(dcApplication()) << "Starting" << application.applicationName() << application.applicationVersion();
    qCDebug(dcApplication()) << "==========================================================";
    if (parser.isSet(developmentOption)) {
        qCWarning(dcApplication()) << "##########################################################";
        qCWarning(dcApplication()) << "#                   DEVELOPMENT MODE                     #";
        qCWarning(dcApplication()) << "##########################################################";
    }

    if (s_loggingEnabled)
        qCDebug(dcApplication()) << "Logging enabled. Writing logs to" << s_logFile.fileName();

    Authenticator *authenticator = nullptr;
    if (parser.isSet(mockAuthenticatorOption)) {
        authenticator = qobject_cast<Authenticator *>(new DummyAuthenticator(nullptr));
    } else {
        if (configuration->proxyEnabled()) {
            // Create default authenticator
            authenticator = qobject_cast<Authenticator *>(new AwsAuthenticator(configuration->awsCredentialsUrl(), nullptr));
        }
    }

    // Configure and start the engines
    Engine::instance()->setAuthenticator(authenticator);
    Engine::instance()->setDeveloperModeEnabled(parser.isSet(developmentOption));
    Engine::instance()->start(configuration);

    return application.exec();
}
