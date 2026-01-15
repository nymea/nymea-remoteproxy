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

#include "tunnelproxye2ee.h"

#include <QtEndian>

#include <cstring>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

namespace remoteproxyclient {

namespace {

const quint8 kE2eeMagic = 0xEE;
const quint8 kE2eeVersion = 1;

const quint8 kE2eeTypeClientHello = 0x01;
const quint8 kE2eeTypeServerHello = 0x02;
const quint8 kE2eeTypeData = 0x10;

const quint32 kE2eeMaxPayload = 1024 * 1024;
const int kE2eeHeaderSize = 8;
const int kE2eeHelloSize = 64;

const int kE2eeKeySize = 32;
const int kE2eeNonceSize = 12;
const int kE2eeTagSize = 16;
const int kE2eeMaxPlaintext = 16 * 1024;

const char kE2eeHkdfInfo[] = "nymea-remoteproxy-tunnel-e2ee-v1";

QString opensslErrorString()
{
    unsigned long err = ERR_get_error();
    if (err == 0)
        return QString();

    char buffer[256];
    ERR_error_string_n(err, buffer, sizeof(buffer));
    return QString::fromLatin1(buffer);
}

bool hmacSha256(const QByteArray &key, const QByteArray &data, QByteArray *out, QString *error)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen = 0;
    if (!HMAC(EVP_sha256(),
              reinterpret_cast<const unsigned char *>(key.constData()),
              key.size(),
              reinterpret_cast<const unsigned char *>(data.constData()),
              data.size(),
              digest,
              &digestLen)) {
        if (error)
            *error = QString("HMAC failed: %1").arg(opensslErrorString());
        return false;
    }

    *out = QByteArray(reinterpret_cast<const char *>(digest), static_cast<int>(digestLen));
    return true;
}

}

TunnelProxyE2ee::TunnelProxyE2ee(Role role) :
    m_role(role),
    m_state(role == RoleClient ? StateIdle : StateAwaitClientHello)
{

}

bool TunnelProxyE2ee::startClientHandshake(QByteArray *outFrame, QString *error)
{
    if (m_role != RoleClient) {
        if (error)
            *error = "Client handshake requested in server role.";

        return false;
    }

    if (m_state != StateIdle) {
        if (error)
            *error = "Handshake already started.";

        return false;
    }

    if (!ensureKeyPair(error))
        return false;

    if (!randomBytes(kE2eeKeySize, &m_clientNonce, error))
        return false;

    QByteArray payload;
    payload.reserve(kE2eeHelloSize);
    payload.append(m_publicKey);
    payload.append(m_clientNonce);

    if (payload.size() != kE2eeHelloSize) {
        if (error)
            *error = "Invalid client hello payload size.";
        return false;
    }

    *outFrame = buildFrame(kE2eeTypeClientHello, payload);
    m_state = StateAwaitServerHello;
    return true;
}

bool TunnelProxyE2ee::processIncoming(const QByteArray &data, QList<QByteArray> *outPlaintexts, QList<QByteArray> *outFrames, QString *error)
{
    m_buffer.append(data);

    while (true) {
        Frame frame;
        QString parseError;
        if (!parseFrame(&frame, &parseError)) {
            if (!parseError.isEmpty()) {
                if (error)
                    *error = parseError;

                return false;
            }
            break;
        }

        switch (frame.type) {
        case kE2eeTypeClientHello:
            if (!handleClientHello(frame, outFrames, error))
                return false;

            break;
        case kE2eeTypeServerHello:
            if (!handleServerHello(frame, error))
                return false;

            break;
        case kE2eeTypeData:
            if (!handleDataFrame(frame, outPlaintexts, error))
                return false;

            break;
        default:
            if (error)
                *error = "Unknown E2EE frame type.";

            return false;
        }
    }

    return true;
}

bool TunnelProxyE2ee::buildDataFrames(const QByteArray &plaintext, QList<QByteArray> *outFrames, QString *error)
{
    if (m_state != StateEstablished) {
        if (error)
            *error = "E2EE session not established.";

        return false;
    }

    int offset = 0;
    while (offset < plaintext.size()) {
        int chunkSize = qMin(kE2eeMaxPlaintext, plaintext.size() - offset);
        QByteArray chunk = plaintext.mid(offset, chunkSize);

        QByteArray nonce = buildNonce(m_sendNonceBase, m_sendCounter);
        if (nonce.size() != kE2eeNonceSize) {
            if (error)
                *error = "Invalid nonce size.";

            return false;
        }

        QByteArray ciphertext;
        QByteArray tag;
        if (!aesGcmEncrypt(m_sendKey, nonce, chunk, &ciphertext, &tag, error))
            return false;

        if (tag.size() != kE2eeTagSize) {
            if (error)
                *error = "Invalid AEAD tag size.";
            return false;
        }

        QByteArray payload = ciphertext + tag;
        outFrames->append(buildFrame(kE2eeTypeData, payload));

        m_sendCounter++;
        offset += chunkSize;
    }

    return true;
}

bool TunnelProxyE2ee::established() const
{
    return m_state == StateEstablished;
}

void TunnelProxyE2ee::reset()
{
    m_buffer.clear();
    clearKeyMaterial();
    m_state = (m_role == RoleClient) ? StateIdle : StateAwaitClientHello;
}

bool TunnelProxyE2ee::parseFrame(Frame *frame, QString *error)
{
    if (m_buffer.size() < kE2eeHeaderSize)
        return false;

    const unsigned char *raw = reinterpret_cast<const unsigned char *>(m_buffer.constData());
    if (raw[0] != kE2eeMagic) {
        if (error)
            *error = "Invalid E2EE magic.";

        return false;
    }
    if (raw[1] != kE2eeVersion) {
        if (error)
            *error = "Unsupported E2EE version.";

        return false;
    }

    quint8 type = raw[2];
    quint32 payloadLen = qFromBigEndian<quint32>(raw + 4);
    if (payloadLen > kE2eeMaxPayload) {
        if (error)
            *error = "E2EE payload size exceeds limit.";

        return false;
    }

    if (m_buffer.size() < kE2eeHeaderSize + static_cast<int>(payloadLen))
        return false;

    frame->type = type;
    frame->payload = m_buffer.mid(kE2eeHeaderSize, payloadLen);
    m_buffer.remove(0, kE2eeHeaderSize + static_cast<int>(payloadLen));
    return true;
}

QByteArray TunnelProxyE2ee::buildFrame(quint8 type, const QByteArray &payload)
{
    QByteArray frame;
    frame.resize(kE2eeHeaderSize);
    frame[0] = static_cast<char>(kE2eeMagic);
    frame[1] = static_cast<char>(kE2eeVersion);
    frame[2] = static_cast<char>(type);
    frame[3] = 0;

    quint32 payloadLen = static_cast<quint32>(payload.size());
    quint32 payloadLenBe = qToBigEndian(payloadLen);
    memcpy(frame.data() + 4, &payloadLenBe, sizeof(payloadLenBe));

    frame.append(payload);
    return frame;
}

bool TunnelProxyE2ee::handleClientHello(const Frame &frame, QList<QByteArray> *outFrames, QString *error)
{
    if (m_role != RoleServer || m_state != StateAwaitClientHello) {
        if (error)
            *error = "Unexpected client hello.";

        return false;
    }

    if (frame.payload.size() != kE2eeHelloSize) {
        if (error)
            *error = "Invalid client hello payload size.";

        return false;
    }

    m_peerPublicKey = frame.payload.left(kE2eeKeySize);
    m_clientNonce = frame.payload.mid(kE2eeKeySize, kE2eeKeySize);

    if (!ensureKeyPair(error))
        return false;

    if (!randomBytes(kE2eeKeySize, &m_serverNonce, error))
        return false;

    if (!deriveSessionKeys(error))
        return false;

    QByteArray payload;
    payload.reserve(kE2eeHelloSize);
    payload.append(m_publicKey);
    payload.append(m_serverNonce);

    outFrames->append(buildFrame(kE2eeTypeServerHello, payload));
    m_state = StateEstablished;
    return true;
}

bool TunnelProxyE2ee::handleServerHello(const Frame &frame, QString *error)
{
    if (m_role != RoleClient || m_state != StateAwaitServerHello) {
        if (error)
            *error = "Unexpected server hello.";

        return false;
    }

    if (frame.payload.size() != kE2eeHelloSize) {
        if (error)
            *error = "Invalid server hello payload size.";

        return false;
    }

    m_peerPublicKey = frame.payload.left(kE2eeKeySize);
    m_serverNonce = frame.payload.mid(kE2eeKeySize, kE2eeKeySize);

    if (!deriveSessionKeys(error))
        return false;

    m_state = StateEstablished;
    return true;
}

bool TunnelProxyE2ee::handleDataFrame(const Frame &frame, QList<QByteArray> *outPlaintexts, QString *error)
{
    if (m_state != StateEstablished) {
        if (error)
            *error = "Encrypted data received before handshake.";

        return false;
    }

    if (frame.payload.size() < kE2eeTagSize) {
        if (error)
            *error = "Invalid encrypted payload size.";

        return false;
    }

    QByteArray ciphertext = frame.payload.left(frame.payload.size() - kE2eeTagSize);
    QByteArray tag = frame.payload.right(kE2eeTagSize);

    QByteArray nonce = buildNonce(m_recvNonceBase, m_recvCounter);
    if (nonce.size() != kE2eeNonceSize) {
        if (error)
            *error = "Invalid nonce size.";

        return false;
    }

    QByteArray plaintext;
    if (!aesGcmDecrypt(m_recvKey, nonce, ciphertext, tag, &plaintext, error))
        return false;

    outPlaintexts->append(plaintext);
    m_recvCounter++;
    return true;
}

bool TunnelProxyE2ee::ensureKeyPair(QString *error)
{
    if (!m_privateKey.isEmpty() && !m_publicKey.isEmpty())
        return true;

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
    if (!ctx) {
        if (error)
            *error = QString("Keygen init failed: %1").arg(opensslErrorString());

        return false;
    }

    EVP_PKEY *pkey = nullptr;
    bool ok = (EVP_PKEY_keygen_init(ctx) == 1) && (EVP_PKEY_keygen(ctx, &pkey) == 1);
    EVP_PKEY_CTX_free(ctx);

    if (!ok || !pkey) {
        if (error)
            *error = QString("Keygen failed: %1").arg(opensslErrorString());

        if (pkey)
            EVP_PKEY_free(pkey);

        return false;
    }

    size_t pubLen = 0;
    size_t privLen = 0;
    if (EVP_PKEY_get_raw_public_key(pkey, nullptr, &pubLen) != 1 ||
            EVP_PKEY_get_raw_private_key(pkey, nullptr, &privLen) != 1) {
        if (error)
            *error = QString("Key export failed: %1").arg(opensslErrorString());

        EVP_PKEY_free(pkey);
        return false;
    }

    QByteArray publicKey(static_cast<int>(pubLen), 0);
    QByteArray privateKey(static_cast<int>(privLen), 0);
    if (EVP_PKEY_get_raw_public_key(pkey, reinterpret_cast<unsigned char *>(publicKey.data()), &pubLen) != 1 ||
            EVP_PKEY_get_raw_private_key(pkey, reinterpret_cast<unsigned char *>(privateKey.data()), &privLen) != 1) {

        if (error)
            *error = QString("Key export failed: %1").arg(opensslErrorString());

        EVP_PKEY_free(pkey);
        return false;
    }

    EVP_PKEY_free(pkey);

    m_publicKey = publicKey;
    m_privateKey = privateKey;
    return true;
}

bool TunnelProxyE2ee::deriveSessionKeys(QString *error)
{
    if (m_privateKey.size() != kE2eeKeySize || m_peerPublicKey.size() != kE2eeKeySize) {
        if (error)
            *error = "Invalid key sizes for key derivation.";

        return false;
    }

    if (m_clientNonce.size() != kE2eeKeySize || m_serverNonce.size() != kE2eeKeySize) {
        if (error)
            *error = "Invalid nonce sizes for key derivation.";

        return false;
    }

    QByteArray secret;
    if (!deriveSharedSecret(m_privateKey, m_peerPublicKey, &secret, error))
        return false;

    QByteArray salt = m_clientNonce + m_serverNonce;
    QByteArray info = QByteArray(kE2eeHkdfInfo, static_cast<int>(sizeof(kE2eeHkdfInfo) - 1));
    QByteArray okm;
    int okmSize = kE2eeKeySize * 2 + kE2eeNonceSize * 2;
    if (!hkdfSha256(salt, secret, info, okmSize, &okm, error))
        return false;

    if (okm.size() != okmSize) {
        if (error)
            *error = "HKDF output size mismatch.";

        return false;
    }

    QByteArray key1 = okm.mid(0, kE2eeKeySize);
    QByteArray key2 = okm.mid(kE2eeKeySize, kE2eeKeySize);
    QByteArray nonce1 = okm.mid(kE2eeKeySize * 2, kE2eeNonceSize);
    QByteArray nonce2 = okm.mid(kE2eeKeySize * 2 + kE2eeNonceSize, kE2eeNonceSize);

    if (m_role == RoleClient) {
        m_sendKey = key1;
        m_recvKey = key2;
        m_sendNonceBase = nonce1;
        m_recvNonceBase = nonce2;
    } else {
        m_sendKey = key2;
        m_recvKey = key1;
        m_sendNonceBase = nonce2;
        m_recvNonceBase = nonce1;
    }

    m_sendCounter = 0;
    m_recvCounter = 0;
    return true;
}

bool TunnelProxyE2ee::randomBytes(int size, QByteArray *out, QString *error)
{
    QByteArray buffer(size, 0);
    if (RAND_bytes(reinterpret_cast<unsigned char *>(buffer.data()), size) != 1) {
        if (error)
            *error = QString("Random generation failed: %1").arg(opensslErrorString());

        return false;
    }

    *out = buffer;
    return true;
}

bool TunnelProxyE2ee::deriveSharedSecret(const QByteArray &privateKey, const QByteArray &peerPublicKey, QByteArray *secret, QString *error)
{
    EVP_PKEY *privKey = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr,
                                                     reinterpret_cast<const unsigned char *>(privateKey.constData()),
                                                     privateKey.size());
    if (!privKey) {
        if (error)
            *error = QString("Private key init failed: %1").arg(opensslErrorString());

        return false;
    }

    EVP_PKEY *peerKey = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr,
                                                    reinterpret_cast<const unsigned char *>(peerPublicKey.constData()),
                                                    peerPublicKey.size());
    if (!peerKey) {
        if (error)
            *error = QString("Peer key init failed: %1").arg(opensslErrorString());

        EVP_PKEY_free(privKey);
        return false;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privKey, nullptr);
    if (!ctx) {
        if (error)
            *error = QString("Derive ctx init failed: %1").arg(opensslErrorString());

        EVP_PKEY_free(peerKey);
        EVP_PKEY_free(privKey);
        return false;
    }

    size_t secretLen = 0;
    bool ok = (EVP_PKEY_derive_init(ctx) == 1) &&
            (EVP_PKEY_derive_set_peer(ctx, peerKey) == 1) &&
            (EVP_PKEY_derive(ctx, nullptr, &secretLen) == 1);

    if (!ok || secretLen == 0) {
        if (error)
            *error = QString("Derive init failed: %1").arg(opensslErrorString());

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerKey);
        EVP_PKEY_free(privKey);
        return false;
    }

    QByteArray secretBuffer(static_cast<int>(secretLen), 0);
    if (EVP_PKEY_derive(ctx, reinterpret_cast<unsigned char *>(secretBuffer.data()), &secretLen) != 1) {
        if (error)
            *error = QString("Derive failed: %1").arg(opensslErrorString());

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(peerKey);
        EVP_PKEY_free(privKey);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(peerKey);
    EVP_PKEY_free(privKey);

    secretBuffer.resize(static_cast<int>(secretLen));
    *secret = secretBuffer;
    return true;
}

bool TunnelProxyE2ee::hkdfSha256(const QByteArray &salt, const QByteArray &ikm, const QByteArray &info, int outLen, QByteArray *okm, QString *error)
{
    QByteArray prk;
    if (!hmacSha256(salt, ikm, &prk, error))
        return false;

    QByteArray result;
    result.reserve(outLen);

    QByteArray t;
    int counter = 1;
    while (result.size() < outLen) {
        QByteArray input = t + info + QByteArray(1, static_cast<char>(counter));
        if (!hmacSha256(prk, input, &t, error))
            return false;

        result.append(t);
        counter++;
        if (counter > 255) {
            if (error)
                *error = "HKDF output too large.";

            return false;
        }
    }

    result.truncate(outLen);
    *okm = result;
    return true;
}

bool TunnelProxyE2ee::aesGcmEncrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &plaintext, QByteArray *ciphertext, QByteArray *tag, QString *error)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        if (error)
            *error = "Cipher init failed.";

        return false;
    }

    bool ok = true;
    int len = 0;
    int outLen = 0;

    ok = (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1);
    ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) == 1);
    ok = ok && (EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                                  reinterpret_cast<const unsigned char *>(key.constData()),
                                  reinterpret_cast<const unsigned char *>(nonce.constData())) == 1);

    if (!ok) {
        if (error)
            *error = QString("Cipher init failed: %1").arg(opensslErrorString());

        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    QByteArray cipherBuf(plaintext.size(), 0);
    if (!plaintext.isEmpty()) {
        ok = (EVP_EncryptUpdate(ctx,
                                reinterpret_cast<unsigned char *>(cipherBuf.data()),
                                &len,
                                reinterpret_cast<const unsigned char *>(plaintext.constData()),
                                plaintext.size()) == 1);
        outLen = len;
    }

    if (!ok || EVP_EncryptFinal_ex(ctx,
                                  reinterpret_cast<unsigned char *>(cipherBuf.data()) + outLen,
                                  &len) != 1) {
        if (error)
            *error = QString("Cipher encrypt failed: %1").arg(opensslErrorString());

        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    outLen += len;
    cipherBuf.resize(outLen);

    QByteArray tagBuf(kE2eeTagSize, 0);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kE2eeTagSize, tagBuf.data()) != 1) {
        if (error)
            *error = QString("Cipher tag failed: %1").arg(opensslErrorString());

        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    EVP_CIPHER_CTX_free(ctx);

    *ciphertext = cipherBuf;
    *tag = tagBuf;
    return true;
}

bool TunnelProxyE2ee::aesGcmDecrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &ciphertext, const QByteArray &tag, QByteArray *plaintext, QString *error)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        if (error)
            *error = "Cipher init failed.";

        return false;
    }

    bool ok = true;
    int len = 0;
    int outLen = 0;

    ok = (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1);
    ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) == 1);
    ok = ok && (EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                                  reinterpret_cast<const unsigned char *>(key.constData()),
                                  reinterpret_cast<const unsigned char *>(nonce.constData())) == 1);

    if (!ok) {
        if (error)
            *error = QString("Cipher init failed: %1").arg(opensslErrorString());

        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    QByteArray plainBuf(ciphertext.size(), 0);
    if (!ciphertext.isEmpty()) {
        ok = (EVP_DecryptUpdate(ctx,
                                reinterpret_cast<unsigned char *>(plainBuf.data()),
                                &len,
                                reinterpret_cast<const unsigned char *>(ciphertext.constData()),
                                ciphertext.size()) == 1);
        outLen = len;
    }

    if (!ok || EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(),
                                   const_cast<char *>(tag.constData())) != 1) {
        if (error)
            *error = QString("Cipher decrypt setup failed: %1").arg(opensslErrorString());

        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(plainBuf.data()) + outLen, &len) != 1) {
        if (error)
            *error = "Cipher authentication failed.";

        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    outLen += len;
    plainBuf.resize(outLen);
    EVP_CIPHER_CTX_free(ctx);

    *plaintext = plainBuf;
    return true;
}

QByteArray TunnelProxyE2ee::buildNonce(const QByteArray &base, quint64 counter)
{
    if (base.size() != kE2eeNonceSize)
        return QByteArray();

    QByteArray nonce = base;
    for (int i = 0; i < 8; i++) {
        int shift = 56 - (i * 8);
        nonce[kE2eeNonceSize - 8 + i] = static_cast<char>((counter >> shift) & 0xFF);
    }
    return nonce;
}

void TunnelProxyE2ee::clearKeyMaterial()
{
    if (!m_privateKey.isEmpty())
        OPENSSL_cleanse(m_privateKey.data(), m_privateKey.size());

    if (!m_sendKey.isEmpty())
        OPENSSL_cleanse(m_sendKey.data(), m_sendKey.size());

    if (!m_recvKey.isEmpty())
        OPENSSL_cleanse(m_recvKey.data(), m_recvKey.size());

    m_privateKey.clear();
    m_publicKey.clear();
    m_peerPublicKey.clear();
    m_clientNonce.clear();
    m_serverNonce.clear();
    m_sendKey.clear();
    m_recvKey.clear();
    m_sendNonceBase.clear();
    m_recvNonceBase.clear();
    m_sendCounter = 0;
    m_recvCounter = 0;
}

}
