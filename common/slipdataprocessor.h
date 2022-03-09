/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
