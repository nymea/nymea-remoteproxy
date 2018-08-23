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

#include <QUrl>
#include <QFileInfo>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "monitor.h"

int main(int argc, char *argv[])
{

    QCoreApplication application(argc, argv);
    application.setApplicationName(SERVER_NAME_STRING);
    application.setOrganizationName("nymea");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy monitor allowes to monitor the live server activity on the a local instance.\n\n"
                                             "Server version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 2018 Simon Stürz <simon.stuerz@guh.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9)));


    QCommandLineOption socketOption(QStringList() << "s" << "socket", "The socket descriptor for the nymea-remoteproxy monitor socket. Default is /tmp/nymea-remoteproxy-monitor.sock", "socket");
    socketOption.setDefaultValue("/tmp/nymea-remoteproxy-monitor.sock");
    parser.addOption(socketOption);
    parser.process(application);

    // Check socket file
    QFileInfo fileInfo(parser.value(socketOption));
    if (!fileInfo.exists()) {
        qWarning() << "Could not find socket descriptor" << fileInfo.canonicalFilePath();
        exit(1);
    }

    if (!fileInfo.isReadable()) {
        qWarning() << "Could not open socket descriptor" << fileInfo.canonicalFilePath();
        exit(1);
    }

    Monitor monitor(parser.value(socketOption));

    return application.exec();
}
