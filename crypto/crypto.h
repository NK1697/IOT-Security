#pragma once

#include <string>

std::string sha256(const std::string& filename);
std::string signData(const std::string& data);
bool verifySignature(const std::string& data, const std::string& signature);