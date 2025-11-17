// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SLIPDATAPROCESSOR_H
#define SLIPDATAPROCESSOR_H

#include <QObject>
#include <QDataStream>

// SLIP: https://tools.ietf.org/html/rfc1055

class SlipDataProcessor
{
    Q_GADGET

public:
    enum ProtocolByte {
        ProtocolByteEnd = 0xC0,
        ProtocolByteEsc = 0xDB,
        ProtocolByteTransposedEnd = 0xDC,
        ProtocolByteTransposedEsc = 0xDD
    };
    Q_ENUM(ProtocolByte)

    typedef struct Frame {
        quint16 socketAddress;
        QByteArray data;
    } Frame;

    explicit SlipDataProcessor() = default;

    static QByteArray deserializeData(const QByteArray &data);
    static QByteArray serializeData(const QByteArray &data);

    static Frame parseFrame(const QByteArray &data);
    static QByteArray buildFrame(const Frame &frame);

};

#endif // SLIPDATAPROCESSOR_H
