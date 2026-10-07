/**
 * @file gamepad_input_state.ino
 * @example gamepad_input_state.ino
 * @brief Example of using gamepad::input::State
 */

#include "gamepad_input_state.h"

using namespace gamepad::input;

void setup() {
  Serial.begin(115200);

  State state;
  Serial.print("Default button states: ");
  Serial.println(state.buttons);

  for (size_t i = 0; i < kAxisCount; ++i) {
    Serial.print("Default axis ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(state.axes[i]);
  }

  state[Axis::kLeftStickX] = 50;
  state[Axis::kLeftStickY] = 100;
  state[Axis::kRightStickX] = 150;
  state[Axis::kRightStickY] = 200;

  Serial.print("After set and get axes: ");
  Serial.print(state[Axis::kLeftStickX]);
  Serial.print(", ");
  Serial.print(state[Axis::kLeftStickY]);
  Serial.print(", ");
  Serial.print(state[Axis::kRightStickX]);
  Serial.print(", ");
  Serial.println(state[Axis::kRightStickY]);

  Serial.print("Button Start state: ");
  Serial.println(state[Button::kStart]);

  state.Set(Button::kStart);
  Serial.print("After set and get button Start state: ");
  Serial.println(state[Button::kStart]);

  if (state[Button::kStart]) {
    Serial.println("Button Start is pressed");
  } else {
    Serial.println("Button Start is not pressed");
  }

  Serial.print("raw data: ");
  for (size_t i = 0; i < sizeof(state); ++i) {
    Serial.print(reinterpret_cast<const uint8_t*>(&state)[i], 16);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {}