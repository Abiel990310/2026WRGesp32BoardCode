#include "ble_debug.h"
#include <NimBLEDevice.h>
#include <cstring>
namespace {
NimBLEServer* server = nullptr;
NimBLECharacteristic* text = nullptr;
bool enabled = false;
}
void bleDebugSetup() {
  // Same FF10/FF11 protocol as the existing Mac robot-debug application.
  server = NimBLEDevice::createServer();
  auto service = server->createService(NimBLEUUID(uint16_t(0xFF10)));
  text = service->createCharacteristic(NimBLEUUID(uint16_t(0xFF11)), NIMBLE_PROPERTY::NOTIFY, 192);
  service->start(); server->start(); server->advertiseOnDisconnect(false);
  auto adv = NimBLEDevice::getAdvertising();
  adv->setName("TUMi-Debug"); adv->addServiceUUID(NimBLEUUID(uint16_t(0xFF10)));
  adv->enableScanResponse(true);
}
void bleDebugSetEnabled(bool value) {
  enabled = value;
  server->advertiseOnDisconnect(value);
  if (value) NimBLEDevice::getAdvertising()->start();
  else {
    NimBLEDevice::getAdvertising()->stop();
    for (auto handle : server->getPeerDevices()) server->disconnect(handle);
  }
}
void bleDebugPublish(const char* line) {
  if (!enabled || !line) return;
  const size_t length = strlen(line);
  for (auto handle : server->getPeerDevices()) {
    const size_t mtu = server->getPeerMTU(handle);
    if (mtu <= 3) continue;
    for (size_t offset = 0; offset < length; offset += mtu - 3) {
      const size_t count = (length-offset < mtu-3) ? length-offset : mtu-3;
      if (!text->notify(reinterpret_cast<const uint8_t*>(line+offset), count, handle)) break;
    }
  }
}
