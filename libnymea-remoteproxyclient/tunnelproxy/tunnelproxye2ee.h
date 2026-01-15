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

#ifndef TUNNELPROXYE2EE_H
#define TUNNELPROXYE2EE_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QtGlobal>

namespace remoteproxyclient {

class TunnelProxyE2ee
{
public:
    enum Role {
        RoleClient,
        RoleServer
    };

    explicit TunnelProxyE2ee(Role role);

    bool startClientHandshake(QByteArray *outFrame, QString *error);
    bool processIncoming(const QByteArray &data, QList<QByteArray> *outPlaintexts, QList<QByteArray> *outFrames, QString *error);
    bool buildDataFrames(const QByteArray &plaintext, QList<QByteArray> *outFrames, QString *error);

    bool established() const;
    void reset();

private:
    enum State {
        StateIdle,
        StateAwaitClientHello,
        StateAwaitServerHello,
        StateEstablished
    };

    struct Frame {
        quint8 type = 0;
        QByteArray payload;
    };

    Role m_role;
    State m_state;

    QByteArray m_buffer;

    QByteArray m_privateKey;
    QByteArray m_publicKey;
    QByteArray m_peerPublicKey;
    QByteArray m_clientNonce;
    QByteArray m_serverNonce;

    QByteArray m_sendKey;
    QByteArray m_recvKey;
    QByteArray m_sendNonceBase;
    QByteArray m_recvNonceBase;
    quint64 m_sendCounter = 0;
    quint64 m_recvCounter = 0;

    bool parseFrame(Frame *frame, QString *error);
    static QByteArray buildFrame(quint8 type, const QByteArray &payload);

    bool handleClientHello(const Frame &frame, QList<QByteArray> *outFrames, QString *error);
    bool handleServerHello(const Frame &frame, QString *error);
    bool handleDataFrame(const Frame &frame, QList<QByteArray> *outPlaintexts, QString *error);

    bool ensureKeyPair(QString *error);
    bool deriveSessionKeys(QString *error);

    static bool randomBytes(int size, QByteArray *out, QString *error);
    static bool deriveSharedSecret(const QByteArray &privateKey, const QByteArray &peerPublicKey, QByteArray *secret, QString *error);
    static bool hkdfSha256(const QByteArray &salt, const QByteArray &ikm, const QByteArray &info, int outLen, QByteArray *okm, QString *error);
    static bool aesGcmEncrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &plaintext, QByteArray *ciphertext, QByteArray *tag, QString *error);
    static bool aesGcmDecrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &ciphertext, const QByteArray &tag, QByteArray *plaintext, QString *error);
    static QByteArray buildNonce(const QByteArray &base, quint64 counter);

    void clearKeyMaterial();
};

}

#endif // TUNNELPROXYE2EE_H
