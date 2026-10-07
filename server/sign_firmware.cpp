#include "ota_server.h"
#include <iostream>

using namespace std;

int main() {
    try {
        createManifest(
            "firmware/v1.bin",
            "server/v1.manifest",
            "FACTORY-MCU-V1",
            1
        );

        createManifest(
            "firmware/v2.bin",
            "server/v2.manifest",
            "FACTORY-MCU-V1",
            2
        );
    }
    catch (const exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}