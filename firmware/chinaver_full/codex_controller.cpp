#include "codex_controller.h"
#include "fleet_config.h"
namespace {
CodexPad vendor;
CodexController controllerState;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
gamepad::input::State received;
uint32_t sequence = 0, consumed = 0, retryAt = 0;
bool attempted = false;
void (*onUp)(ControllerPtr);
void (*onDown)(ControllerPtr);
void resetCapture() {
  portENTER_CRITICAL(&mux); received.Reset(); sequence = 0; portEXIT_CRITICAL(&mux);
  consumed = 0; controllerState.inputs.Reset(); controllerState.fresh = false;
}
bool subscribe() {
  auto client = vendor.ble_client();
  auto service = client ? client->getService(uint16_t(0xFFA0)) : nullptr;
  auto input = service ? service->getCharacteristic(uint16_t(0xFFA1)) : nullptr;
  return input && input->canNotify() && input->subscribe(true,
    [](NimBLERemoteCharacteristic*, uint8_t* data, size_t length, bool) {
      if (length != sizeof(gamepad::input::State)) return;
      const auto state = gamepad::input::State::FromBytes(data);
      if (state.buttons & ~uint32_t(0x1FFFF)) return;
      portENTER_CRITICAL(&mux); received = state; ++sequence; portEXIT_CRITICAL(&mux);
    });
}
}
void codexSetup(void (*connected)(ControllerPtr), void (*disconnected)(ControllerPtr)) {
  onUp = connected; onDown = disconnected; vendor.Init(); resetCapture();
}
void codexSuspend() {
  if (controllerState.connected) { controllerState.connected = false; onDown(&controllerState); }
  if (vendor.is_connected()) vendor.Disconnect();
  resetCapture();
}
void codexUpdate() {
  controllerState.fresh = false;
  vendor.Update();
  if (fleetInhibited()) { codexSuspend(); return; }
  if (!vendor.is_connected()) {
    if (controllerState.connected) {
      controllerState.connected = false; onDown(&controllerState); resetCapture();
      attempted = true; retryAt = millis(); return; // stop before any blocking reconnect
    }
    if (attempted && uint32_t(millis()-retryAt) < 3000) return;
    resetCapture();
    if (vendor.Connect(fleetControllerMac.c_str(), 2000) && subscribe()) {
      controllerState.connected = true; onUp(&controllerState);
    } else vendor.Disconnect();
    attempted = true; retryAt = millis();
  }
  if (!controllerState.connected) return;
  portENTER_CRITICAL(&mux);
  const auto state = received; const uint32_t count = sequence;
  portEXIT_CRITICAL(&mux);
  if (count != consumed) {
    consumed = count; controllerState.inputs = state; controllerState.fresh = true;
  }
  // Input-on-change protocol: keep the last state while the real BLE link is up.
  // Never synthesize a fresh packet or use notification silence as a disconnect.
}
