#pragma once

enum BootState {
    NORMAL,
    TESTING
};

class Device {
private:
    char activeBank;
    BootState bootState;
    int confirmedVersion;

public:
    Device();

    void loadState();
    void saveState();

    char getActiveBank();
    void setActiveBank(char bank);

    BootState getBootState();
    void setBootState(BootState state);

    int getVersion();
    void setVersion(int version);
    void reset();
    void rollback();
};