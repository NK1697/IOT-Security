#include "crypto.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/evp.h>
#include <openssl/pem.h>

using namespace std;

string bytesToHex(const unsigned char* data, size_t size) {
    stringstream ss;

    for (size_t i = 0; i < size; i++)
        ss << hex << setw(2) << setfill('0') << (int)data[i];

    return ss.str();
}

vector<unsigned char> hexToBytes(const string& hex) {
    if (hex.size() % 2)
        throw runtime_error("Invalid hexadecimal string");

    vector<unsigned char> bytes;

    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned int value;
        stringstream ss(hex.substr(i, 2));

        ss >> std::hex >> value;

        if (ss.fail())
            throw runtime_error("Invalid hexadecimal data");

        bytes.push_back((unsigned char)value);
    }

    return bytes;
}

string sha256(const string& filename) {
    ifstream file(filename, ios::binary);

    if (!file)
        throw runtime_error("Could not open firmware file");

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx)
        throw runtime_error("Could not create hash context");

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("SHA-256 initialization failed");
    }

    char buffer[4096];

    while (file) {
        file.read(buffer, sizeof(buffer));
        streamsize bytes = file.gcount();

        if (bytes > 0)
            EVP_DigestUpdate(ctx, buffer, bytes);
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length;

    EVP_DigestFinal_ex(ctx, hash, &length);
    EVP_MD_CTX_free(ctx);

    return bytesToHex(hash, length);
}

string signData(const string& data) {
    FILE* file = fopen("keys/vendor_private.pem", "rb");

    if (!file)
        throw runtime_error("Could not open private key");

    EVP_PKEY* key = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);
    fclose(file);

    if (!key)
        throw runtime_error("Could not read private key");

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx) {
        EVP_PKEY_free(key);
        throw runtime_error("Could not create signing context");
    }

    if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, key) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw runtime_error("Signing initialization failed");
    }

    size_t length = 0;

    EVP_DigestSign(
        ctx,
        nullptr,
        &length,
        (const unsigned char*)data.data(),
        data.size()
    );

    vector<unsigned char> signature(length);

    if (EVP_DigestSign(
            ctx,
            signature.data(),
            &length,
            (const unsigned char*)data.data(),
            data.size()) != 1) {

        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw runtime_error("Signing failed");
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);

    return bytesToHex(signature.data(), length);
}

bool verifySignature(const string& data, const string& signature) {
    FILE* file = fopen("keys/vendor_public.pem", "rb");

    if (!file)
        throw runtime_error("Could not open public key");

    EVP_PKEY* key = PEM_read_PUBKEY(file, nullptr, nullptr, nullptr);
    fclose(file);

    if (!key)
        throw runtime_error("Could not read public key");

    vector<unsigned char> sig = hexToBytes(signature);

    if (sig.size() != 64) {
        EVP_PKEY_free(key);
        return false;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx) {
        EVP_PKEY_free(key);
        throw runtime_error("Could not create verification context");
    }

    if (EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, key) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw runtime_error("Verification initialization failed");
    }

    int result = EVP_DigestVerify(
        ctx,
        sig.data(),
        sig.size(),
        (const unsigned char*)data.data(),
        data.size()
    );

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);

    return result == 1;
}