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

#ifndef LOGENGINE_H
#define LOGENGINE_H

#include <QFile>
#include <QObject>

#include "tunnelconnection.h"

namespace remoteproxy {

class LogEngine : public QObject
{
    Q_OBJECT
public:
    explicit LogEngine(QObject *parent = nullptr);
    ~LogEngine();

    void logTunnel(const TunnelConnection &tunnel);
    void logStatistics(int tunnelCount, int connectionCount, int troughput);

private:
    QFile m_tunnelsFile;
    QFile m_statisticsFile;

    QString m_tunnelsFileName;
    QString m_statisticsFileName;

    bool m_enabled = false;
    int m_currentDay;

    void rotateLogs();
    QString createTimestamp();

public slots:
    void enable();
    void disable();

};

}

#endif // LOGENGINE_H
