#include "../include/engine.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string jsonValue(const std::string& line, const std::string& key) {
    const std::string marker = "\"" + key + "\"";
    const std::size_t key_pos = line.find(marker);
    if (key_pos == std::string::npos) {
        return "";
    }

    const std::size_t colon_pos = line.find(':', key_pos + marker.size());
    if (colon_pos == std::string::npos) {
        return "";
    }

    const std::size_t value_start = line.find('"', colon_pos + 1);
    if (value_start == std::string::npos) {
        return "";
    }

    const std::size_t value_end = line.find('"', value_start + 1);
    if (value_end == std::string::npos) {
        return "";
    }

    return line.substr(value_start + 1, value_end - value_start - 1);
}

void writeError(const std::string& message) {
    std::cout << "{\"ok\":false,\"error\":\"" << message << "\"}\n";
}

void handleGenerateKey() {
    std::vector<std::uint8_t> key(crypto::KEY_BYTES);
    crypto::generateKey(key.data());

    std::cout << "{\"ok\":true,\"key\":\"" << crypto::toHex(key.data(), key.size()) << "\"}\n";
    crypto::zeroMemory(key.data(), key.size());
}

void handleGenerateNonce() {
    std::vector<std::uint8_t> nonce(crypto::NONCE_BYTES);
    crypto::generateNonce(nonce.data());

    std::cout << "{\"ok\":true,\"nonce\":\"" << crypto::toHex(nonce.data(), nonce.size()) << "\"}\n";
}

void handleEncrypt(const std::string& line) {
    std::vector<std::uint8_t> key = crypto::fromHex(jsonValue(line, "key"));
    std::vector<std::uint8_t> nonce = crypto::fromHex(jsonValue(line, "nonce"));
    std::vector<std::uint8_t> plaintext = crypto::fromBase64(jsonValue(line, "data"));

    if (key.size() != crypto::KEY_BYTES) {
        throw std::runtime_error("invalid key length");
    }
    if (nonce.size() != crypto::NONCE_BYTES) {
        throw std::runtime_error("invalid nonce length");
    }

    std::vector<std::uint8_t> ciphertext(plaintext.size() + crypto::MAC_BYTES);
    std::size_t ciphertext_len = 0;

    if (!crypto::encryptData(plaintext.data(), plaintext.size(), key.data(), nonce.data(), ciphertext.data(), ciphertext_len)) {
        throw std::runtime_error("encryption failed");
    }

    ciphertext.resize(ciphertext_len);

    std::cout << "{\"ok\":true,\"ciphertext\":\""
              << crypto::toBase64(ciphertext.data(), ciphertext.size())
              << "\"}\n";

    crypto::zeroMemory(key.data(), key.size());
    crypto::zeroMemory(plaintext.data(), plaintext.size());
}

void handleDecrypt(const std::string& line) {
    std::vector<std::uint8_t> key = crypto::fromHex(jsonValue(line, "key"));
    std::vector<std::uint8_t> nonce = crypto::fromHex(jsonValue(line, "nonce"));
    std::vector<std::uint8_t> ciphertext = crypto::fromBase64(jsonValue(line, "data"));

    if (key.size() != crypto::KEY_BYTES) {
        throw std::runtime_error("invalid key length");
    }
    if (nonce.size() != crypto::NONCE_BYTES) {
        throw std::runtime_error("invalid nonce length");
    }
    if (ciphertext.size() < crypto::MAC_BYTES) {
        throw std::runtime_error("ciphertext too short");
    }

    std::vector<std::uint8_t> plaintext(ciphertext.size() - crypto::MAC_BYTES);
    std::size_t plaintext_len = 0;

    if (!crypto::decryptData(ciphertext.data(), ciphertext.size(), key.data(), nonce.data(), plaintext.data(), plaintext_len)) {
        throw std::runtime_error("decryption failed");
    }

    plaintext.resize(plaintext_len);

    std::cout << "{\"ok\":true,\"plaintext\":\""
              << crypto::toBase64(plaintext.data(), plaintext.size())
              << "\"}\n";

    crypto::zeroMemory(key.data(), key.size());
    crypto::zeroMemory(plaintext.data(), plaintext.size());
}

void handleLine(const std::string& line) {
    const std::string op = jsonValue(line, "op");

    try {
        if (op == "generate_key") {
            handleGenerateKey();
        } else if (op == "generate_nonce") {
            handleGenerateNonce();
        } else if (op == "encrypt") {
            handleEncrypt(line);
        } else if (op == "decrypt") {
            handleDecrypt(line);
        } else {
            writeError("unknown operation");
        }
    } catch (const std::exception& err) {
        writeError(err.what());
    }
}

} // namespace

int main() {
    if (!crypto::init()) {
        std::cerr << "failed to initialize libsodium\n";
        return 1;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            handleLine(line);
            std::cout << std::flush;
        }
    }

    return 0;
}
