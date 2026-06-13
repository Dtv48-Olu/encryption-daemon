#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace crypto {

// libsodium XChaCha20-Poly1305 constants.
// This is authenticated encryption, not AES.
constexpr std::size_t KEY_BYTES = 32;   // 256-bit symmetric key
constexpr std::size_t NONCE_BYTES = 24; // 192-bit nonce for XChaCha20-Poly1305
constexpr std::size_t MAC_BYTES = 16;   // Authentication tag size

bool init();

void generateKey(std::uint8_t* out_key);
void generateNonce(std::uint8_t* out_nonce);

bool encryptData(const std::uint8_t* plaintext,
                 std::size_t plaintext_len,
                 const std::uint8_t* key,
                 const std::uint8_t* nonce,
                 std::uint8_t* out_ciphertext,
                 std::size_t& out_ciphertext_len);

bool decryptData(const std::uint8_t* ciphertext,
                 std::size_t ciphertext_len,
                 const std::uint8_t* key,
                 const std::uint8_t* nonce,
                 std::uint8_t* out_plaintext,
                 std::size_t& out_plaintext_len);

void zeroMemory(void* ptr, std::size_t len);

std::string toHex(const std::uint8_t* bytes, std::size_t len);
std::vector<std::uint8_t> fromHex(const std::string& hex);

std::string toBase64(const std::uint8_t* bytes, std::size_t len);
std::vector<std::uint8_t> fromBase64(const std::string& base64);

} // namespace crypto
