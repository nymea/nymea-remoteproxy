/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
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

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDateTime>

class Utils {

public:
    Utils() = default;

    inline static QString getDurationString(uint timestamp) {
        uint duration = QDateTime::currentDateTimeUtc().toTime_t() - timestamp;
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
