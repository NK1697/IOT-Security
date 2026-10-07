#include "ota_server.h"
#include "../crypto/crypto.h"
#include <fstream>
#include <iostream>

using namespace std;

void createManifest(
    const string& firmware,
    const string& output,
    const string& hardwareID,
    int version
) {
    string hash = sha256(firmware);

    string data =
        hardwareID + "|" +
        to_string(version) + "|" +
        hash;

    string signature = signData(data);

    ofstream file(output);

    if (!file)
        throw runtime_error("Could not create manifest");

    file << "hardwareID=" << hardwareID << "\n";
    file << "version=" << version << "\n";
    file << "sha256=" << hash << "\n";
    file << "signature=" << signature << "\n";

    cout << firmware << " -> " << output << "\n";
}