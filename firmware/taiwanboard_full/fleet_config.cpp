#include "fleet_config.h"
#include "fleet_validation.h"
#include <Preferences.h>
#include <esp_mac.h>

String fleetControllerMac;
namespace {
char chip[18], nonce[17] = {}, staged[18] = {}, buffer[160];
const char* variantName;
size_t used = 0;
bool overflow = false, locked = false;
uint32_t bootAt, rebootAt = 0;
String loadAddress() {
  Preferences p;
  if (!p.begin("codex-fleet", true)) return "";
  String value = p.getString("controller", "");
  p.end();
  char normalized[18];
  return fleetNormalizeMac(value.c_str(), normalized) ? String(normalized) : String();
}
void info(const char* request) {
  Serial.printf("FLEET INFO %s %s %s %s 1.0.0\n", request, chip, variantName,
                fleetControllerMac.isEmpty() ? "NONE" : fleetControllerMac.c_str());
}
void command(char* line, void (*stopMotors)()) {
  char* save = nullptr;
  char* fields[6] = {};
  int count = 0;
  for (char* s = strtok_r(line, " ", &save); s && count < 6; s = strtok_r(nullptr, " ", &save)) fields[count++] = s;
  if (count < 3 || strcmp(fields[0], "FLEET") || !fleetValidNonce(fields[2])) return;
  const char* request = fields[2];
  if (count == 3 && !strcmp(fields[1], "HELLO")) { info(request); return; }
  if (count < 4 || strcmp(fields[3], chip)) {
    Serial.printf("FLEET ERROR %s WRONG_BOARD\n", request); return;
  }
  if (count == 4 && !strcmp(fields[1], "BEGIN")) {
    locked = true; staged[0] = 0; strlcpy(nonce, request, sizeof(nonce));
    stopMotors();
    Serial.printf("FLEET LOCKED %s %s\n", nonce, chip); return;
  }
  if (!locked || strcmp(nonce, request)) {
    Serial.printf("FLEET ERROR %s NO_SESSION\n", request); return;
  }
  stopMotors();
  if (count == 5 && !strcmp(fields[1], "STAGE")) {
    char normalized[18];
    if (!fleetNormalizeMac(fields[4], normalized)) {
      staged[0] = 0; Serial.printf("FLEET ERROR %s BAD_ADDRESS\n", nonce); return;
    }
    strlcpy(staged, normalized, sizeof(staged));
    Serial.printf("FLEET STAGED %s %s\n", nonce, staged); return;
  }
  if (count == 4 && !strcmp(fields[1], "COMMIT") && staged[0]) {
    Preferences p;
    bool ok = p.begin("codex-fleet", false);
    if (ok) { ok = p.putString("controller", staged) == strlen(staged); p.end(); }
    fleetControllerMac = loadAddress();
    if (!ok || fleetControllerMac != staged) {
      Serial.printf("FLEET ERROR %s NVS_VERIFY\n", nonce); return;
    }
    Serial.printf("FLEET SAVED %s %s %s %s\n", nonce, chip, variantName, staged); return;
  }
  if (count == 4 && !strcmp(fields[1], "REBOOT")) {
    Serial.printf("FLEET REBOOTING %s\n", nonce);
    rebootAt = millis() + 100; return;
  }
  Serial.printf("FLEET ERROR %s BAD_COMMAND\n", nonce);
}
}
void fleetSetup(const char* variant) {
  variantName = variant;
  uint8_t mac[6]; esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(chip, sizeof(chip), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  fleetControllerMac = loadAddress(); bootAt = millis();
  Serial.printf("FLEET BOOT %s %s %s\n", chip, variantName,
                fleetControllerMac.isEmpty() ? "NONE" : fleetControllerMac.c_str());
  Serial.println("USB provisioning window: 8 seconds. Unassigned robots stay disabled.");
}
bool fleetInhibited() {
  return locked || fleetControllerMac.isEmpty() || uint32_t(millis() - bootAt) < 8000;
}
bool fleetService(void (*stopMotors)()) {
  for (int budget = 0; budget < 96 && Serial.available(); ++budget) {
    const char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (!overflow && used) { buffer[used] = 0; command(buffer, stopMotors); }
      used = 0; overflow = false;
    } else if (used < sizeof(buffer)-1) buffer[used++] = c;
    else overflow = true;
  }
  if (rebootAt && int32_t(millis()-rebootAt) >= 0) { stopMotors(); ESP.restart(); }
  return fleetInhibited();
}
