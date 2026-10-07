#pragma once

#include "../device/device.h"
#include <string>

struct Manifest {
    std::string hardwareID;
    int version;
    std::string expectedHash;
    std::string signature;
};

struct Firmware {
    std::string name;
    std::string file;
    Manifest manifest;
};

class Bootloader {
private:
    Firmware bankA;
    Firmware bankB;
    Device device;

    const std::string expectedHardwareID = "FACTORY-MCU-V1";

    Firmware* getBank(char bank);
    char getInactiveBank();

    Manifest loadManifest(const std::string& filename);
    bool verifyFirmware(Firmware& firmware);

public:
    Bootloader(Firmware a, Firmware b);

    void boot();
    void update(bool simulateFailure);
    void confirmBoot();
    void reset();
    void loadBankAManifest(const std::string& filename);
    void loadBankBManifest(const std::string& filename);
};