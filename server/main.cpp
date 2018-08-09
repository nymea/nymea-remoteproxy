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
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QSslConfiguration>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "loggingcategories.h"
#include "authentication/awsauthenticator.h"

using namespace remoteproxy;

static QHash<QString, bool> s_loggingFilters;

static QFile s_logFile;
static bool s_loggingEnabled = false;

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
        category->setEnabled(QtWarningMsg, s_loggingFilters.value("Warnings"));
    }
}

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

    QCoreApplication application(argc, argv);
    application.setApplicationName("nymea-remoteproxy");
    application.setOrganizationName("guh");
    application.setApplicationVersion("0.0.1");

    s_loggingFilters.insert("Engine", true);
    s_loggingFilters.insert("Application", true);
    s_loggingFilters.insert("JsonRpc", true);
    s_loggingFilters.insert("JsonRpcTraffic", true);
    s_loggingFilters.insert("WebSocketServer", true);
    s_loggingFilters.insert("WebSocketServerTraffic", false);
    s_loggingFilters.insert("Authenticator", true);
    s_loggingFilters.insert("ProxyServer", true);

    // command line parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy server. This server allowes nymea-cloud users and "
                                             "registered nymea deamons to establish a tunnel connection.\n\n"
                                             "Copyright %1 2018 Simon Stürz <simon.stuerz@guh.io>").arg(QChar(0xA9)));

    QCommandLineOption logfileOption(QStringList() << "l" << "logging", "Write log file to the given logfile.",
                                     "logfile", "/var/log/nymea-remoteproxy.log");
    parser.addOption(logfileOption);

    QCommandLineOption serverOption(QStringList() << "s" << "server", "The server address this proxy will listen on. "
                                                                      "Default is 127.0.0.1", "hostaddress", "127.0.0.1");
    parser.addOption(serverOption);

    QCommandLineOption portOption(QStringList() << "p" << "port", "The proxy server port. Default is 1212", "port", "1212");
    parser.addOption(portOption);

    QCommandLineOption certOption(QStringList() << "c" <<"certificate", "The path to the SSL certificate used for "
                                                                        "this proxy server.", "certificate");
    parser.addOption(certOption);

    QCommandLineOption certKeyOption(QStringList() << "k" << "certificate-key", "The path to the SSL certificate key "
                                                                                "used for this proxy server.", "certificate-key");
    parser.addOption(certKeyOption);

    QCommandLineOption authenticationUrlOption(QStringList() << "a" << "authentication-server",
                                               "The server url of the AWS authentication server.", "url", "https://127.0.0.1");
    parser.addOption(authenticationUrlOption);

    QCommandLineOption verboseOption(QStringList() << "v" << "verbose", "Print more verbose.");
    parser.addOption(verboseOption);

    parser.process(application);

    if (parser.isSet(verboseOption)) {
        s_loggingFilters["Debug"] = true;
        s_loggingFilters["WebSocketServerTraffic"] = true;
    }

    QLoggingCategory::installFilter(loggingCategoryFilter);

    // Open the logfile, if any specified
    if (parser.isSet(logfileOption)) {
        QFileInfo fi(parser.value(logfileOption));
        QDir logDir(fi.absolutePath());
        if (!logDir.exists() && !logDir.mkpath(logDir.absolutePath())) {
            qCCritical(dcApplication()) << "Error opening log file" << parser.value(logfileOption);
            return 1;
        }
        s_logFile.setFileName(parser.value(logfileOption));
        if (!s_logFile.open(QFile::WriteOnly | QFile::Append)) {
            qCCritical(dcApplication()) << "Error opening log file" << parser.value(logfileOption);
            return 1;
        }
        s_loggingEnabled = true;
    }

    // Proxy server host address
    QHostAddress serverHostAddress = QHostAddress(parser.value(serverOption));
    if (serverHostAddress.isNull()) {
        qCCritical(dcApplication()) << "Invalid hostaddress for the proxy server:" << parser.value(serverOption);
        exit(-1);
    }

    // Port
    bool ok = false;
    uint port = parser.value(portOption).toUInt(&ok);
    if (!ok) {
        qCCritical(dcApplication()) << "Invalid port value:" << parser.value(portOption);
        exit(-1);
    }

    if (port > 65535) {
        qCCritical(dcApplication()) << "Port value is out of range:" << parser.value(portOption);
        exit(-1);
    }

    // SSL certificate
    QSslConfiguration sslConfiguration;
    if (parser.isSet(certOption)) {
        // Load certificate
        QFile certFile(parser.value(certOption));
        if (!certFile.open(QIODevice::ReadOnly)) {
            qCCritical(dcApplication()) << "Could not open certificate file:" << parser.value(certOption) << certFile.errorString();
            exit(-1);
        }

        QSslCertificate certificate(&certFile, QSsl::Pem);
        qCDebug(dcApplication()) << "Loaded successfully certificate" << parser.value(certOption);
        certFile.close();

        // Create SSL configuration
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
        sslConfiguration.setLocalCertificate(certificate);
        sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);
    }

    // SSL key
    if (parser.isSet(certKeyOption)) {
        QFile certKeyFile(parser.value(certKeyOption));
        if (!certKeyFile.open(QIODevice::ReadOnly)) {
            qCCritical(dcApplication()) << "Could not open certificate key file:" << parser.value(certKeyOption) << certKeyFile.errorString();
            exit(-1);
        }

        QSslKey sslKey(&certKeyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        qCDebug(dcApplication()) << "Loaded successfully certificate key" << parser.value(certKeyOption);
        certKeyFile.close();
        sslConfiguration.setPrivateKey(sslKey);
    }

    if (sslConfiguration.isNull()) {
        qCCritical(dcApplication()) << "No SSL configuration specified. The server does not suppoert insecure connections.";
        exit(-1);
    }

    // Authentication server url
    QUrl authenticationServerUrl(parser.value(authenticationUrlOption));
    if (!authenticationServerUrl.isValid()) {
        qCCritical(dcApplication()) << "Invalid authentication server url:" << parser.value(authenticationUrlOption);
        exit(-1);
    }

    qCDebug(dcApplication()) << "==============================================";
    qCDebug(dcApplication()) << "Starting" << application.applicationName() << application.applicationVersion();
    qCDebug(dcApplication()) << "==============================================";

    if (s_loggingEnabled)
        qCDebug(dcApplication()) << "Logging enabled. Writing logs to" << s_logFile.fileName();

    // Create default authenticator
    AwsAuthenticator *authenticator = new AwsAuthenticator(nullptr);

    // Configure and start the engines
    Engine::instance()->setAuthenticator(authenticator);
    Engine::instance()->setWebSocketServerHostAddress(serverHostAddress);
    Engine::instance()->setWebSocketServerPort(static_cast<quint16>(port));
    Engine::instance()->setSslConfiguration(sslConfiguration);
    Engine::instance()->setAuthenticationServerUrl(authenticationServerUrl);
    Engine::instance()->start();

    return application.exec();
}
