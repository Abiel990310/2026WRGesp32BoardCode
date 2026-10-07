/**
 * @file gamepad_input_tracker.ino
 * @example gamepad_input_tracker.ino
 * @brief Example of using gamepad::input::Tracker
 */

#include "gamepad_input_tracker.h"

using namespace gamepad::input;

void setup() {
  Serial.begin(115200);

  Tracker tracker;
  State state;
  state.Set(Button::kStart);

  tracker.Tick();
  tracker.Update(state);
  Serial.println(tracker.pressed(Button::kStart));

  tracker.Tick();
  Serial.println(tracker.pressed(Button::kStart));
}

void loop() {}