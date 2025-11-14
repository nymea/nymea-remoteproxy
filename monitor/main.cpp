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
#include <QFileInfo>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "monitor.h"
#include "noninteractivemonitor.h"
#include "../version.h"

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
                                             "Copyright %3 %4 nymea GmbH <developer@nymea.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9))
                                     .arg(COPYRIGHT_YEAR_STRING));


    QCommandLineOption socketOption(QStringList() << "s" << "socket", "The socket descriptor for the nymea-remoteproxy monitor socket. Default is /tmp/nymea-remoteproxy-monitor.sock", "socket");
    socketOption.setDefaultValue("/tmp/nymea-remoteproxy-monitor.sock");
    parser.addOption(socketOption);

    QCommandLineOption noninteractiveOption(QStringList() << "n" << "non-interactive", "Connect to the server and list information and connections and exit.");
    parser.addOption(noninteractiveOption);

    QCommandLineOption allOption(QStringList() << "a" << "all", "Show all connections, not only the active tunnels. Currently only available with the non-interactive mode.");
    parser.addOption(allOption);

    QCommandLineOption jsonOption(QStringList() << "j" << "json", "Connect to the server and print the raw json data.");
    parser.addOption(jsonOption);

    parser.process(application);

    // Check socket file
    QFileInfo fileInfo(parser.value(socketOption));
    if (!fileInfo.exists()) {
        qWarning() << "Could not find socket descriptor" << parser.value(socketOption);
        exit(EXIT_FAILURE);
    }

    if (!fileInfo.isReadable()) {
        qWarning() << "Could not open socket descriptor" << parser.value(socketOption);
        exit(EXIT_FAILURE);
    }

    if (parser.isSet(noninteractiveOption) || parser.isSet(jsonOption)) {
        NonInteractiveMonitor *monitor = new NonInteractiveMonitor(parser.value(socketOption), parser.isSet(jsonOption), parser.isSet(allOption), &application);
        Q_UNUSED(monitor);
    } else {
        if (parser.isSet(allOption)) {
            qWarning() << "Error: The \"all\" option is only available with the non-interavtice mode.";
            exit(EXIT_FAILURE);
        }

        Monitor *monitor = new Monitor(parser.value(socketOption), parser.isSet(jsonOption), &application);
        Q_UNUSED(monitor);
    }

    return application.exec();
}
