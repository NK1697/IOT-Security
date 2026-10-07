#include "bootloader.h"
#include "../crypto/crypto.h"
#include "../device/device.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

Bootloader::Bootloader(Firmware a, Firmware b)
    : bankA(a), bankB(b) {}

Firmware* Bootloader::getBank(char bank) {
    return bank == 'A' ? &bankA : &bankB;
}

char Bootloader::getInactiveBank() {
    return device.getActiveBank() == 'A' ? 'B' : 'A';
}

Manifest Bootloader::loadManifest(const string& filename) {
    ifstream file(filename);

    if (!file)
        throw runtime_error("Could not open manifest");

    Manifest manifest{};
    string line;

    while (getline(file, line)) {
        size_t pos = line.find('=');

        if (pos == string::npos)
            continue;

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (key == "hardwareID")
            manifest.hardwareID = value;
        else if (key == "version")
            manifest.version = stoi(value);
        else if (key == "sha256")
            manifest.expectedHash = value;
        else if (key == "signature")
            manifest.signature = value;
    }

    if (manifest.hardwareID.empty() ||
        manifest.version <= 0 ||
        manifest.expectedHash.empty() ||
        manifest.signature.empty()) {
        throw runtime_error("Invalid manifest");
    }

    return manifest;
}

bool Bootloader::verifyFirmware(Firmware& firmware) {
    cout << "\nVerifying " << firmware.name << "...\n";

    if (firmware.manifest.hardwareID != expectedHardwareID) {
        cout << "HARDWARE ID MISMATCH\n";
        return false;
    }

    cout << "Hardware ID verified\n";

    string data =
        firmware.manifest.hardwareID + "|" +
        to_string(firmware.manifest.version) + "|" +
        firmware.manifest.expectedHash;

    if (!verifySignature(data, firmware.manifest.signature)) {
        cout << "SIGNATURE INVALID\n";
        return false;
    }

    cout << "SIGNATURE VALID\n";

    string hash = sha256(firmware.file);

    cout << "Calculated SHA-256: " << hash << "\n";
    cout << "Manifest SHA-256:   "
         << firmware.manifest.expectedHash << "\n";

    if (hash != firmware.manifest.expectedHash) {
        cout << "HASH MISMATCH\n";
        return false;
    }

    cout << "HASH MATCH\n";

    return true;
}

void Bootloader::boot() {
    Firmware* firmware =
        getBank(device.getActiveBank());

    cout << "\nBootloader started\n";
    cout << "Active bank: "
         << device.getActiveBank() << "\n";
    cout << "Firmware: "
         << firmware->name << "\n";
    cout << "Version: "
         << firmware->manifest.version << "\n";

    if (firmware->manifest.version < device.getVersion()) {
        cout << "ANTI-ROLLBACK CHECK FAILED\n";
        return;
    }

    if (verifyFirmware(*firmware))
        cout << "BOOTING FIRMWARE\n";
    else
        cout << "BOOT ABORTED\n";
}

void Bootloader::update(bool simulateFailure) {
    char candidateBank = getInactiveBank();
    Firmware* candidate = getBank(candidateBank);

    cout << "\nChecking for update...\n";
    cout << "Inactive bank: " << candidateBank << "\n";

    if (candidate->manifest.version <= device.getVersion()) {
        cout << "No newer firmware available\n";
        return;
    }

    cout << "New firmware found: v"
         << candidate->manifest.version << "\n";

    if (!verifyFirmware(*candidate)) {
        cout << "Firmware update rejected\n";
        return;
    }

    cout << "Firmware update verified\n";

    device.setActiveBank(candidateBank);
    device.setBootState(TESTING);
    device.saveState();

    cout << "Starting trial boot of Bank "
         << candidateBank << "...\n";

    if (simulateFailure) {
        cout << "Simulated firmware failure\n";
        return;
    }

    confirmBoot();
}

void Bootloader::confirmBoot() {
    if (device.getBootState() != TESTING) {
        cout << "No firmware waiting for confirmation\n";
        return;
    }

    Firmware* firmware =
        getBank(device.getActiveBank());

    if (!verifyFirmware(*firmware)) {
        cout << "Trial firmware verification failed\n";
        device.rollback();
        return;
    }

    device.setVersion(firmware->manifest.version);
    device.setBootState(NORMAL);
    device.saveState();

    cout << "Bank "
         << device.getActiveBank()
         << " confirmed\n";

    cout << "Version "
         << device.getVersion()
         << " is now active\n";
}

void Bootloader::loadBankAManifest(const string& filename) {
    bankA.manifest = loadManifest(filename);
}

void Bootloader::loadBankBManifest(const string& filename) {
    bankB.manifest = loadManifest(filename);
}

void Bootloader::reset() {
    cout << "RESET";
    device.reset();
    boot();
}

int main() {
    try {
        Firmware firmwareV1 = {
            "Factory Controller v1",
            "firmware/v1.bin",
            {"", 0, "", ""}
        };

        Firmware firmwareV2 = {
            "Factory Controller v2",
            "firmware/v2.bin",
            {"", 0, "", ""}
        };

        Bootloader bootloader(firmwareV1, firmwareV2);

        bootloader.loadBankAManifest(
            "server/v1.manifest"
        );

        bootloader.loadBankBManifest(
            "server/v2.manifest"
        );

        bootloader.boot();

        cout << "\nTESTING ROLLBACK\n";
        bootloader.update(true);
        bootloader.reset();

        cout << "\nTESTING SUCCESSFUL UPDATE\n";
        bootloader.update(false);
        bootloader.boot();
    }
    catch (const exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}