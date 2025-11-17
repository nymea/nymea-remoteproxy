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

#ifndef UTILS_H
#define UTILS_H

#include <QTextStream>
#include <QString>
#include <QDateTime>

inline QTextStream& qStdOut() {
    static QTextStream ts(stdout);
    return ts;
}

class Utils {

public:
    Utils() = default;

    inline static QString getDurationString(uint timestamp) {
        uint duration = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() - timestamp;
        int seconds = static_cast<int>(duration % 60);
        duration /= 60;
        int minutes = static_cast<int>(duration % 60);
        duration /= 60;
        int hours = static_cast<int>(duration % 24);
        return QString::asprintf("%02d:%02d:%02d", hours, minutes, seconds);
    }

    inline static QString humanReadableTraffic(int bytes) {
        double dataCount = bytes;
        QStringList list;
        list << "KB" << "MB" << "GB" << "TB";

        QStringListIterator i(list);
        QString unit("B");

        while(dataCount >= 1024.0 && i.hasNext()) {
            unit = i.next();
            dataCount /= 1024.0;
        }
        return QString().setNum(dataCount,'f',2) + " " + unit;
    }
};

#endif // UTILS_H
