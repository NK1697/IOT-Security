#include "device.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

Device::Device() {
    activeBank = 'A';
    bootState = NORMAL;
    confirmedVersion = 0;
    loadState();
}

void Device::loadState() {
    ifstream file("device_data/boot_state.txt");

    if (!file) {
        saveState();
        return;
    }

    int state;

    file >> activeBank;
    file >> state;
    file >> confirmedVersion;

    if (activeBank != 'A' && activeBank != 'B')
        throw runtime_error("Invalid active bank");

    if (state != NORMAL && state != TESTING)
        throw runtime_error("Invalid boot state");

    bootState = static_cast<BootState>(state);

    if (bootState == TESTING) {
        cout << "Previous boot failed during testing\n";
        rollback();
    }
}

void Device::saveState() {
    ofstream file("device_data/boot_state.txt");

    if (!file)
        throw runtime_error("Could not save device state");

    file << activeBank << "\n";
    file << bootState << "\n";
    file << confirmedVersion << "\n";
}

char Device::getActiveBank() {
    return activeBank;
}

void Device::setActiveBank(char bank) {
    activeBank = bank;
}

BootState Device::getBootState() {
    return bootState;
}

void Device::setBootState(BootState state) {
    bootState = state;
}

int Device::getVersion() {
    return confirmedVersion;
}

void Device::setVersion(int version) {
    confirmedVersion = version;
}

void Device::rollback() {
    cout << "Rolling back to Bank A\n";

    activeBank = 'A';
    bootState = NORMAL;

    saveState();
}

void Device::reset() {
    loadState();
}