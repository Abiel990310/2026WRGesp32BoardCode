#pragma once
#include <Arduino.h>
#include <codex_pad.h>
#include "fleet_validation.h"

constexpr uint8_t DPAD_LEFT = 8;
class CodexController {
 public:
  gamepad::input::State inputs;
  bool connected = false, fresh = false;
  bool isConnected() const { return connected; }
  bool hasData() const { return fresh; }
  int axisX() const { return fleetAxis(inputs.axes[0]); }
  int axisY() const { return -fleetAxis(inputs.axes[1]); }
  int axisRX() const { return fleetAxis(inputs.axes[2]); }
  int axisRY() const { return -fleetAxis(inputs.axes[3]); }
  bool a() const { return inputs[gamepad::input::Button::kCrossA]; }
  bool b() const { return inputs[gamepad::input::Button::kCircleB]; }
  bool x() const { return inputs[gamepad::input::Button::kSquareX]; }
  bool y() const { return inputs[gamepad::input::Button::kTriangleY]; }
  bool l1() const { return inputs[gamepad::input::Button::kL1]; }
  bool r1() const { return inputs[gamepad::input::Button::kR1]; }
  int brake() const { return inputs[gamepad::input::Button::kL2] ? 1023 : 0; }
  int throttle() const { return inputs[gamepad::input::Button::kR2] ? 1023 : 0; }
  uint32_t buttons() const { return inputs.buttons; }
  uint32_t debugButtons() const {
    // The existing robot debug UI expects Bluepad-style bits, not vendor raw bits.
    using gamepad::input::Button;
    return (a() ? 1u : 0u) | (b() ? 2u : 0u) | (x() ? 4u : 0u) | (y() ? 8u : 0u) |
           (l1() ? 16u : 0u) | (r1() ? 32u : 0u) |
           (inputs[Button::kL2] ? 64u : 0u) | (inputs[Button::kR2] ? 128u : 0u) |
           (inputs[Button::kL3] ? 256u : 0u) | (inputs[Button::kR3] ? 512u : 0u);
  }
  uint8_t dpad() const {
    using gamepad::input::Button;
    return (inputs[Button::kUp] ? 1 : 0) | (inputs[Button::kDown] ? 2 : 0) |
           (inputs[Button::kRight] ? 4 : 0) | (inputs[Button::kLeft] ? 8 : 0);
  }
  std::string getModelName() const { return "CodexPad-S10"; }
};
using ControllerPtr = CodexController*;
void codexSetup(void (*connected)(ControllerPtr), void (*disconnected)(ControllerPtr));
void codexUpdate();
void codexSuspend();
