#include "../include/engine.h"

#include <sodium.h>

#include <stdexcept>

namespace crypto {

bool init() {
    return sodium_init() >= 0;
}

void generateKey(std::uint8_t* out_key) {
    randombytes_buf(out_key, KEY_BYTES);
}

void generateNonce(std::uint8_t* out_nonce) {
    randombytes_buf(out_nonce, NONCE_BYTES);
}

bool encryptData(const std::uint8_t* plaintext,
                 std::size_t plaintext_len,
                 const std::uint8_t* key,
                 const std::uint8_t* nonce,
                 std::uint8_t* out_ciphertext,
                 std::size_t& out_ciphertext_len) {
    unsigned long long ciphertext_len = 0;

    const int result = crypto_aead_xchacha20poly1305_ietf_encrypt(
        out_ciphertext,
        &ciphertext_len,
        plaintext,
        static_cast<unsigned long long>(plaintext_len),
        nullptr,
        0,
        nullptr,
        nonce,
        key
    );

    out_ciphertext_len = static_cast<std::size_t>(ciphertext_len);
    return result == 0;
}

bool decryptData(const std::uint8_t* ciphertext,
                 std::size_t ciphertext_len,
                 const std::uint8_t* key,
                 const std::uint8_t* nonce,
                 std::uint8_t* out_plaintext,
                 std::size_t& out_plaintext_len) {
    if (ciphertext_len < MAC_BYTES) {
        out_plaintext_len = 0;
        return false;
    }

    unsigned long long plaintext_len = 0;

    const int result = crypto_aead_xchacha20poly1305_ietf_decrypt(
        out_plaintext,
        &plaintext_len,
        nullptr,
        ciphertext,
        static_cast<unsigned long long>(ciphertext_len),
        nullptr,
        0,
        nonce,
        key
    );

    out_plaintext_len = static_cast<std::size_t>(plaintext_len);
    return result == 0;
}

void zeroMemory(void* ptr, std::size_t len) {
    sodium_memzero(ptr, len);
}

std::string toHex(const std::uint8_t* bytes, std::size_t len) {
    if (len == 0) {
        return "";
    }

    std::string hex(len * 2 + 1, '\0');
    sodium_bin2hex(hex.data(), hex.size(), bytes, len);
    hex.pop_back();
    return hex;
}

std::vector<std::uint8_t> fromHex(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        throw std::runtime_error("hex input has an odd number of characters");
    }

    std::vector<std::uint8_t> bytes(hex.size() / 2);

    if (sodium_hex2bin(bytes.data(), bytes.size(), hex.c_str(), hex.size(), nullptr, nullptr, nullptr) != 0) {
        throw std::runtime_error("invalid hex input");
    }

    return bytes;
}

std::string toBase64(const std::uint8_t* bytes, std::size_t len) {
    if (len == 0) {
        return "";
    }

    const std::size_t encoded_len = sodium_base64_encoded_len(len, sodium_base64_VARIANT_ORIGINAL);
    std::string base64(encoded_len, '\0');

    sodium_bin2base64(base64.data(), base64.size(), bytes, len, sodium_base64_VARIANT_ORIGINAL);

    base64.resize(std::char_traits<char>::length(base64.c_str()));
    return base64;
}

std::vector<std::uint8_t> fromBase64(const std::string& base64) {
    std::vector<std::uint8_t> bytes(base64.size());
    std::size_t decoded_len = 0;

    if (sodium_base642bin(
            bytes.data(),
            bytes.size(),
            base64.c_str(),
            base64.size(),
            nullptr,
            &decoded_len,
            nullptr,
            sodium_base64_VARIANT_ORIGINAL
        ) != 0) {
        throw std::runtime_error("invalid base64 input");
    }

    bytes.resize(decoded_len);
    return bytes;
}

} // namespace crypto
