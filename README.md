# Secure IoT Firmware Update

Software prototype of a secure A/B firmware update system.

## Current implementation

- SHA-256 firmware integrity verification
- Ed25519 firmware signing and verification
- Signed manifests
- Hardware ID verification
- Firmware version checking
- A/B firmware banks
- Persistent boot state
- Trial boot
- Automatic rollback
- Anti-rollback protection

## Build

### Generate manifests

g++ server/sign_firmware.cpp server/ota_server.cpp crypto/crypto.cpp \
-I/opt/homebrew/opt/openssl@3/include \
-L/opt/homebrew/opt/openssl@3/lib \
-lcrypto \
-o sign_firmware

./sign_firmware

### Build bootloader

g++ bootloader/bootloader.cpp device/device.cpp crypto/crypto.cpp \
-I/opt/homebrew/opt/openssl@3/include \
-L/opt/homebrew/opt/openssl@3/lib \
-lcrypto \
-o secure_boot

./secure_boot

## Planned work

- SRAM PUF-based device identity
- Fuzzy extractor
- HKDF key derivation
- Challenge-response attestation
- OTA communication
- Watchdog-based trial boot
- ESP32 implementation