// taiwanboard_full: TUMi / RLISP602ER, current robot
// CodexPad fleet version. Official Espressif ESP32 Dev Module, core 3.3.6.
#include <Arduino.h>
#include "codex_controller.h"
#include "fleet_config.h"
#include "compass_safety.h"
#include <Wire.h>
#include "ble_debug.h"

struct Motor {
  int pinA;
  int pinB;
  int channelA;
  int channelB;
  bool inverted;
};

#include "board_config.h"

constexpr int kPwmFrequency = 5000;
constexpr int kPwmResolution = 8;
constexpr int kDriveTurnMax = 220;    // Slightly slower driving turns; straight speed remains 255.
constexpr int kStickDeadzone = 45;    // Stick values: about -511..512

ControllerPtr controller = nullptr;

constexpr uint8_t kMagAddress = 0x0C;
enum class CompassType { None, GY91, BNO055 };
enum class DriveMode { Manual, Absolute };
CompassType compassType = CompassType::None;
DriveMode driveMode = DriveMode::Manual;
bool returnToZeroActive = false;
int returnToZeroDirection = 0;
uint32_t returnToZeroStartedMs = 0;
uint32_t brakeUntilMs = 0;
constexpr uint32_t kReturnToZeroTimeoutMs = 4000;
constexpr uint32_t kBrakePulseMs = 100;
constexpr int kReturnToZeroPower = 170;
constexpr float kReturnToZeroToleranceDegrees = 1.0f;
constexpr uint32_t kCompassIntervalMs = 10;
constexpr float kAbsoluteSlowTurnDegrees = 10.0f;
constexpr int kAbsoluteSlowTurnPower = 45;
constexpr float kAbsoluteFullTurnDegrees = 70.0f;
constexpr float kAbsoluteForwardBlendDegrees = 45.0f;
constexpr int kTriggerDeadzone = 30;  // CodexPad digital triggers adapted to 0 or 1023.
uint8_t mpuAddress = 0;
uint8_t bnoAddress = 0;
CompassRecovery compassRecovery;
bool compassReady = false;
bool compassHeadingValid = false;
uint32_t compassSampleMs = 0;
bool compassZeroSet = false;
float compassHeadingDegrees = 0.0f;
float compassZeroDegrees = 0.0f;
float magAdjust[3] = {1.0f, 1.0f, 1.0f};
uint32_t lastControllerPacketMs = 0;
uint32_t linkBrakeUntilMs = 0;
bool haveControllerPacket = false;
bool controllerInputArmed = false;
bool controllerFailsafeActive = false;
bool debugMode = false;
bool debugChordWasPressed = false;

float wrap360(float angle) {
  while (angle >= 360.0f) angle -= 360.0f;
  while (angle < 0.0f) angle += 360.0f;
  return angle;
}

float wrap180(float angle) {
  angle = wrap360(angle);
  return angle > 180.0f ? angle - 360.0f : angle;
}

bool i2cPresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool writeI2cRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readI2cRegisters(uint8_t address, uint8_t reg, uint8_t* data, size_t length) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(address, static_cast<uint8_t>(length)) != length) return false;
  for (size_t i = 0; i < length; ++i) data[i] = Wire.read();
  return true;
}

void setupCompass() {
  compassRecovery.attempted(millis());
  compassHeadingValid = false;
  compassReady = false;
  compassType = CompassType::None;
  Wire.begin(kCompassSda, kCompassScl);
  Wire.setClock(400000);
  Wire.setTimeOut(20);

  if (debugMode) Serial.print("Compass I2C probe:");
  const uint8_t expectedAddresses[] = {0x0C, 0x28, 0x29, 0x68, 0x69, 0x76, 0x77};
  bool foundAny = false;
  for (uint8_t address : expectedAddresses) {
    if (i2cPresent(address)) {
      if (debugMode) Serial.printf(" 0x%02X", address);
      foundAny = true;
    }
  }
  if (debugMode) {
    if (!foundAny) Serial.print(" no devices");
    Serial.println();
  }

  // BNO055 uses 0x28 by default, or 0x29 when ADR is high.
  for (uint8_t address : {static_cast<uint8_t>(0x28), static_cast<uint8_t>(0x29)}) {
    uint8_t chipId = 0;
    if (i2cPresent(address) && readI2cRegisters(address, 0x00, &chipId, 1) && chipId == 0xA0) {
      bnoAddress = address;
      if (!writeI2cRegister(bnoAddress, 0x3D, 0x00)) return;  // CONFIGMODE
      delay(25);
      if (!writeI2cRegister(bnoAddress, 0x07, 0x00)) return;  // Register page 0
      if (!writeI2cRegister(bnoAddress, 0x3E, 0x00)) return;  // Normal power mode
      if (!writeI2cRegister(bnoAddress, 0x3F, 0x00)) return;  // Internal oscillator
      if (!writeI2cRegister(bnoAddress, 0x3B, 0x00)) return;  // Euler output in degrees
      delay(10);
      if (!writeI2cRegister(bnoAddress, 0x3D, 0x0C)) return;  // NDOF fusion mode
      delay(30);
      compassType = CompassType::BNO055;
      compassReady = true;
      Serial.printf("BNO055 ready at 0x%02X\n", bnoAddress);
      return;
    }
  }

  if (i2cPresent(0x68)) mpuAddress = 0x68;
  else if (i2cPresent(0x69)) mpuAddress = 0x69;
  else {
    if (debugMode) Serial.println("GY-91 MPU9250 not found at 0x68 or 0x69");
    return;
  }

  // Wake the MPU9250 and enable direct I2C access to its AK8963 magnetometer.
  if (!writeI2cRegister(mpuAddress, 0x6B, 0x00)) return;  // PWR_MGMT_1
  delay(100);
  if (!writeI2cRegister(mpuAddress, 0x6A, 0x00)) return;  // USER_CTRL: disable I2C master
  if (!writeI2cRegister(mpuAddress, 0x37, 0x02)) return;  // INT_PIN_CFG: bypass enable
  delay(10);

  if (!i2cPresent(kMagAddress)) {
    if (debugMode) Serial.println("AK8963 compass not found at 0x0C");
    return;
  }

  // Read factory sensitivity adjustment, then select 16-bit / 100 Hz mode.
  if (!writeI2cRegister(kMagAddress, 0x0A, 0x00)) return;
  delay(10);
  if (!writeI2cRegister(kMagAddress, 0x0A, 0x0F)) return;
  delay(10);
  uint8_t asa[3];
  if (readI2cRegisters(kMagAddress, 0x10, asa, sizeof(asa))) {
    for (int i = 0; i < 3; ++i) {
      magAdjust[i] = ((asa[i] - 128.0f) / 256.0f) + 1.0f;
    }
  }
  if (!writeI2cRegister(kMagAddress, 0x0A, 0x00)) return;
  delay(10);
  if (!writeI2cRegister(kMagAddress, 0x0A, 0x16)) return;
  delay(10);

  compassReady = true;
  compassType = CompassType::GY91;
  Serial.printf("GY-91 compass ready (MPU address 0x%02X)\n", mpuAddress);
}

void compassOffline() {
  if (compassReady) Serial.println("COMPASS OFFLINE: optional compass features disabled; manual driving remains available");
  compassReady = false;
  compassHeadingValid = false;
  compassType = CompassType::None;
  compassRecovery.attempted(millis());
}

void updateCompass() {
  if (compassHeadingValid && millis() - compassSampleMs > 250) compassOffline();
  if (!compassReady) {
    const bool neutral = controller == nullptr || controllerControlsNeutral(controller);
    const bool braking = int32_t(brakeUntilMs-millis()) > 0 || int32_t(linkBrakeUntilMs-millis()) > 0;
    if (compassRecovery.retryAllowed(millis(), neutral, returnToZeroActive, braking)) {
      if (debugMode) Serial.println("Retrying compass detection...");
      setupCompass();
    }
    return;
  }

  static uint32_t lastCompassMs = 0;
  static uint32_t lastCompassPrintMs = 0;
  if (millis() - lastCompassMs < kCompassIntervalMs) return;
  lastCompassMs = millis();

  if (compassType == CompassType::BNO055) {
    uint8_t fusion[2];
    if (!readI2cRegisters(bnoAddress, 0x39, fusion, sizeof(fusion))) { compassOffline(); return; }
    if (fusion[1] != 0) { compassOffline(); return; }
    if (fusion[0] != 5) { compassHeadingValid = false; return; }
    uint8_t euler[6];
    uint8_t calibration = 0;
    if (!readI2cRegisters(bnoAddress, 0x1A, euler, sizeof(euler))) { compassOffline(); return; }
    if (debugMode && !readI2cRegisters(bnoAddress, 0x35, &calibration, 1)) { compassOffline(); return; }
    const int16_t headingRaw = static_cast<int16_t>((euler[1] << 8) | euler[0]);
    const int16_t rollRaw = static_cast<int16_t>((euler[3] << 8) | euler[2]);
    const int16_t pitchRaw = static_cast<int16_t>((euler[5] << 8) | euler[4]);
    compassHeadingDegrees = wrap360(headingRaw / 16.0f);
    compassHeadingValid = true;
    compassSampleMs = millis();
    if (!compassZeroSet) {
      compassZeroDegrees = compassHeadingDegrees;
      compassZeroSet = true;
    }
    if (debugMode && millis() - lastCompassPrintMs >= 200) {
      lastCompassPrintMs = millis();
      Serial.printf("BNO055 heading=%6.1f relative=%6.1f roll=%6.1f pitch=%6.1f deg | CAL sys=%d gyro=%d accel=%d mag=%d\n",
                    compassHeadingDegrees, wrap360(compassHeadingDegrees - compassZeroDegrees),
                    rollRaw / 16.0f, pitchRaw / 16.0f,
                    (calibration >> 6) & 3, (calibration >> 4) & 3,
                    (calibration >> 2) & 3, calibration & 3);
    }
    return;
  }

  uint8_t status;
  if (!readI2cRegisters(kMagAddress, 0x02, &status, 1)) { compassOffline(); return; }
  if (!(status & 0x01)) return;

  uint8_t data[7];
  if (!readI2cRegisters(kMagAddress, 0x03, data, sizeof(data))) { compassOffline(); return; }
  if (data[6] & 0x08) return;  // Magnetic sensor overflow.

  const int16_t rawX = static_cast<int16_t>((data[1] << 8) | data[0]);
  const int16_t rawY = static_cast<int16_t>((data[3] << 8) | data[2]);
  const int16_t rawZ = static_cast<int16_t>((data[5] << 8) | data[4]);
  const float mx = rawX * magAdjust[0] * 0.15f;
  const float my = rawY * magAdjust[1] * 0.15f;
  const float mz = rawZ * magAdjust[2] * 0.15f;
  float heading = atan2f(my, mx) * 180.0f / PI;
  if (heading < 0.0f) heading += 360.0f;
  compassHeadingDegrees = heading;
  compassHeadingValid = true;
  compassSampleMs = millis();
  if (!compassZeroSet) {
    compassZeroDegrees = compassHeadingDegrees;
    compassZeroSet = true;
  }

  if (debugMode && millis() - lastCompassPrintMs >= 200) {
    lastCompassPrintMs = millis();
    Serial.printf("COMPASS X=%7.1f Y=%7.1f Z=%7.1f uT heading=%6.1f deg (uncalibrated)\n",
                  mx, my, mz, heading);
  }
}

void writeMotorPin(const Motor& motor, bool pinA, int duty) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(pinA ? motor.pinA : motor.pinB, duty);
#else
  ledcWrite(pinA ? motor.channelA : motor.channelB, duty);
#endif
}

void driveMotor(int index, int command) {
  Motor& motor = motors[index];
  command = constrain(command, -255, 255);
  if (motor.inverted) {
    command = -command;
  }

  if (command > 0) {
    writeMotorPin(motor, false, 0);
    writeMotorPin(motor, true, command);
  } else if (command < 0) {
    writeMotorPin(motor, true, 0);
    writeMotorPin(motor, false, -command);
  } else {
    writeMotorPin(motor, true, 0);
    writeMotorPin(motor, false, 0);
  }
}

void stopAllMotors() {
  for (int i = 0; i < 4; ++i) {
    driveMotor(i, 0);
  }
}

void brakeAllMotors() {
  // Both inputs HIGH give short braking on the RZ7889 (taiwanboard) and
  // TB67H450 (chinaver). Use only as a short pulse.
  for (const Motor& motor : motors) {
    writeMotorPin(motor, true, 255);
    writeMotorPin(motor, false, 255);
  }
}

bool controllerControlsNeutral(ControllerPtr ctl) {
  return abs(ctl->axisX()) <= kStickDeadzone &&
         abs(ctl->axisY()) <= kStickDeadzone &&
         abs(ctl->axisRX()) <= kStickDeadzone &&
         abs(ctl->axisRY()) <= kStickDeadzone &&
         ctl->throttle() <= kTriggerDeadzone &&
         ctl->brake() <= kTriggerDeadzone &&
         ctl->buttons() == 0;
}

void stopForLostController() {
  returnToZeroActive = false;
  brakeUntilMs = 0;
  controllerInputArmed = false;
  if (!controllerFailsafeActive) {
    controllerFailsafeActive = true;
    linkBrakeUntilMs = millis() + kBrakePulseMs;
    Serial.println("CONTROLLER FAILSAFE: waiting for first input; motors stopped. Release controls to re-arm.");
  }
  if (millis() < linkBrakeUntilMs) brakeAllMotors();
  else stopAllMotors();
}

void onConnectedController(ControllerPtr ctl) {
  if (controller == nullptr) {
    controller = ctl;
    haveControllerPacket = false;
    controllerInputArmed = false;
    controllerFailsafeActive = false;
    linkBrakeUntilMs = 0;
    stopAllMotors();
    Serial.printf("Connected: %s; release controls to arm\n", ctl->getModelName().c_str());
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  if (controller == ctl) {
    stopAllMotors();
    returnToZeroActive = false;
    controllerInputArmed = false;
    haveControllerPacket = false;
    controllerFailsafeActive = false;
    debugChordWasPressed = false;
    linkBrakeUntilMs = millis() + kBrakePulseMs;
    controller = nullptr;
    Serial.println("Controller disconnected; all motors stopped");
  }
}

void setupMotorPwm() {
  for (const Motor& motor : motors) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(motor.pinA, kPwmFrequency, kPwmResolution);
    ledcAttach(motor.pinB, kPwmFrequency, kPwmResolution);
#else
    ledcSetup(motor.channelA, kPwmFrequency, kPwmResolution);
    ledcSetup(motor.channelB, kPwmFrequency, kPwmResolution);
    ledcAttachPin(motor.pinA, motor.channelA);
    ledcAttachPin(motor.pinB, motor.channelB);
#endif
  }
  stopAllMotors();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.printf("%s controller + compass firmware starting\n", kSketchName);
  setupMotorPwm();
  fleetSetup(kSketchName);
  setupCompass();

  codexSetup(&onConnectedController, &onDisconnectedController);
  bleDebugSetup();
  Serial.println("Waiting for this robot\'s assigned CodexPad controller");
}

void loop() {
  if (fleetService(stopAllMotors)) {
    codexSuspend(); stopAllMotors();
    returnToZeroActive = false; brakeUntilMs = linkBrakeUntilMs = 0;
    controllerInputArmed = haveControllerPacket = false;
    updateCompass(); // capture startup zero while the motors remain inhibited
    delay(2); return;
  }
  codexUpdate();

  if (controller == nullptr || !controller->isConnected()) {
    if (millis() < linkBrakeUntilMs) brakeAllMotors();
    else stopAllMotors();
    updateCompass();
    static uint32_t lastDisconnectedDebugMs = 0;
    if (debugMode && millis() - lastDisconnectedDebugMs >= 500) {
      lastDisconnectedDebugMs = millis();
      bleDebugPublish("CONTROLLER DISCONNECTED; motors stopped\n");
    }
    delay(10);
    return;
  }

  if (controller->hasData()) {
    lastControllerPacketMs = millis();
    haveControllerPacket = true;
  }
  if (!haveControllerPacket) {
    stopForLostController();
    updateCompass();
    static uint32_t lastFailsafeDebugMs = 0;
    if (debugMode && millis() - lastFailsafeDebugMs >= 500) {
      lastFailsafeDebugMs = millis();
      bleDebugPublish("FAILSAFE: waiting for first controller input; motors stopped\n");
    }
    delay(10);
    return;
  }

  if (!controllerInputArmed) {
    stopAllMotors();
    if (controllerControlsNeutral(controller)) {
      controllerInputArmed = true;
      controllerFailsafeActive = false;
      Serial.println("CONTROLLER ARMED: input received and controls neutral");
    }
    updateCompass();
    delay(10);
    return;
  }

  updateCompass();
  if (CompassRecovery::requiresManualFallback(driveMode == DriveMode::Absolute, compassHeadingValid, compassZeroSet)) {
    driveMode = DriveMode::Manual;
    returnToZeroActive = false;
    brakeUntilMs = 0;
    controllerInputArmed = false;
    stopAllMotors();
    Serial.println("COMPASS LOST: switched to MANUAL. Release controls to re-arm; absolute mode will not resume automatically.");
    delay(2);
    return;
  }

  // D-pad left + the two thin shoulder buttons (L1/R1) toggles debug output.
  const bool debugChordPressed = (controller->dpad() & DPAD_LEFT) &&
                                 controller->l1() && controller->r1();
  if (debugChordPressed && !debugChordWasPressed) {
    debugMode = !debugMode;
    bleDebugSetEnabled(debugMode);
    Serial.printf("DEBUG %s\n", debugMode ? "ON" : "OFF");
  }
  debugChordWasPressed = debugChordPressed;

  // Circle / B captures the robot's current direction as compass heading 0 degrees.
  static bool circleWasPressed = false;
  const bool circlePressed = controller->b();
  if (circlePressed && !circleWasPressed && compassHeadingValid && millis() - compassSampleMs <= 250) {
    compassZeroDegrees = compassHeadingDegrees;
    compassZeroSet = true;
    returnToZeroActive = false;
    Serial.printf("COMPASS ZERO RESET: absolute %.1f deg is now 0 deg\n", compassZeroDegrees);
  }
  circleWasPressed = circlePressed;

  // Triangle + Cross toggles manual and compass-absolute driving.
  static bool modeChordWasPressed = false;
  const bool modeChordPressed = controller->y() && controller->a();
  if (modeChordPressed && !modeChordWasPressed) {
    if (driveMode == DriveMode::Absolute) driveMode = DriveMode::Manual;
    else if (compassHeadingValid && compassZeroSet) driveMode = DriveMode::Absolute;
    else Serial.println("ABSOLUTE unavailable: compass missing; staying in MANUAL");
    returnToZeroActive = false;
    stopAllMotors();
    Serial.printf("DRIVE MODE: %s\n", driveMode == DriveMode::Manual ? "MANUAL" : "ABSOLUTE");
  }
  modeChordWasPressed = modeChordPressed;

  // Square starts a closed-loop turn back to compass-relative zero.
  static bool squareWasPressed = false;
  const bool squarePressed = controller->x();
  if (squarePressed && !squareWasPressed) {
    if (compassHeadingValid && compassZeroSet) {
      const float currentRelative = wrap360(compassHeadingDegrees - compassZeroDegrees);
      const float initialError = wrap180(-currentRelative);
      if (fabsf(initialError) <= kReturnToZeroToleranceDegrees) {
        returnToZeroActive = false;
        Serial.println("AUTO ZERO: already at 0 deg");
      } else {
        returnToZeroDirection = initialError > 0.0f ? 1 : -1;
        returnToZeroActive = true;
        returnToZeroStartedMs = millis();
        Serial.printf("AUTO ZERO: turn direction=%d, constant power 170, stop at zero crossing\n",
                      returnToZeroDirection);
      }
    } else {
      Serial.println("AUTO ZERO unavailable: waiting for compass");
    }
  }
  squareWasPressed = squarePressed;

  const int rawLeftX = controller->axisX();
  const int rawLeftY = controller->axisY();
  const int rawRightX = controller->axisRX();
  const int rawRightY = controller->axisRY();
  const int rawLeftTrigger = constrain(controller->brake(), 0, 1023);
  const int rawRightTrigger = constrain(controller->throttle(), 0, 1023);
  const int forwardDuty = rawRightTrigger <= kTriggerDeadzone
                              ? 0
                              : map(rawRightTrigger, kTriggerDeadzone, 1023, 0, 255);
  const int reverseDuty = rawLeftTrigger <= kTriggerDeadzone
                              ? 0
                              : map(rawLeftTrigger, kTriggerDeadzone, 1023, 0, 255);
  // Opposing triggers cancel rather than fighting the motors.
  const int driveDuty = forwardDuty - reverseDuty;
  const float relativeHeading = compassZeroSet
                                    ? wrap360(compassHeadingDegrees - compassZeroDegrees)
                                    : 0.0f;
  const bool userStickInput = abs(rawLeftX) > kStickDeadzone ||
                              abs(rawLeftY) > kStickDeadzone ||
                              abs(rawRightX) > kStickDeadzone;
  const bool driverInput = userStickInput ||
                           (driveMode == DriveMode::Absolute && driveDuty != 0);

  // Stick or active trigger input overrides automatic alignment immediately.
  if (returnToZeroActive && driverInput) {
    returnToZeroActive = false;
    Serial.println("AUTO ZERO overridden by driver input");
  }
  if (driverInput) brakeUntilMs = 0;

  int throttle = 0;
  int steering = 0;
  int leftCommand = 0;
  int rightCommand = 0;
  float targetHeading = 0.0f;
  float headingError = 0.0f;

  if (returnToZeroActive) {
    if (!compassHeadingValid || !compassZeroSet || millis() - compassSampleMs > 250) {
      returnToZeroActive = false;
      Serial.println("AUTO ZERO cancelled: compass unavailable");
    } else {
      headingError = wrap180(-relativeHeading);
      const float errorSize = fabsf(headingError);
      const int currentDirection = headingError > 0.0f ? 1 : (headingError < 0.0f ? -1 : 0);
      if (millis() - returnToZeroStartedMs >= kReturnToZeroTimeoutMs) {
        returnToZeroActive = false;
        Serial.println("AUTO ZERO timed out; driver control restored");
      } else if (errorSize <= kReturnToZeroToleranceDegrees ||
                 currentDirection == 0 ||
                 (currentDirection != returnToZeroDirection && errorSize < 90.0f)) {
        // Constant-speed control ends only at the target or on the first sample
        // after crossing it. The active brake removes remaining wheel inertia.
        returnToZeroActive = false;
        brakeUntilMs = millis() + kBrakePulseMs;
        Serial.printf("AUTO ZERO stop at error %.1f deg\n", headingError);
      } else {
        steering = returnToZeroDirection * kReturnToZeroPower;
        leftCommand = steering;
        rightCommand = -steering;
      }
    }
  } else if (driveMode == DriveMode::Manual) {
    // Manual: left-stick Y is throttle; right-stick X is steering.
    if (abs(rawLeftY) > kStickDeadzone) {
      throttle = constrain(map(-rawLeftY, -512, 512, -255, 255), -255, 255);
    }
    int requestedSteering = 0;
    if (abs(rawRightX) > kStickDeadzone) {
      requestedSteering = constrain(map(rawRightX, -512, 512, -255, 255), -255, 255);
    }
    steering = requestedSteering * kDriveTurnMax / 255;
    // Keep steering priority: full right-stick still pivots in place, but slower.
    const int agileThrottle = throttle * (255 - abs(requestedSteering)) / 255;
    leftCommand = agileThrottle + steering;
    rightCommand = agileThrottle - steering;
  } else if (driveMode == DriveMode::Absolute) {
    // Absolute: the left stick always selects the direction of travel. RT
    // drives front-first; LT drives rear-first, so the chassis must face 180
    // degrees away from the selected travel direction when reversing.
    const float stickMagnitude = sqrtf(static_cast<float>(rawLeftX * rawLeftX + rawLeftY * rawLeftY));
    if (stickMagnitude <= kStickDeadzone) {
      // With no direction selected, triggers are plain front/back throttle.
      // This also works when the compass is temporarily unavailable.
      targetHeading = relativeHeading;
      leftCommand = driveDuty;
      rightCommand = driveDuty;
    } else if (compassHeadingValid && compassZeroSet) {
      const float travelHeading = wrap360(
          atan2f(static_cast<float>(rawLeftX), static_cast<float>(-rawLeftY)) * 180.0f / PI);
      targetHeading = wrap360(travelHeading + (driveDuty < 0 ? 180.0f : 0.0f));
      headingError = wrap180(targetHeading - relativeHeading);
      const float errorSize = fabsf(headingError);
      // Continuous turn strength avoids a pivot/drive threshold that can
      // repeatedly stop the robot during S-shaped changes of direction.
      float fullTurnPower;
      if (errorSize <= kAbsoluteSlowTurnDegrees) {
        fullTurnPower = kAbsoluteSlowTurnPower * errorSize / kAbsoluteSlowTurnDegrees;
      } else {
        const float ramp = min((errorSize - kAbsoluteSlowTurnDegrees) /
                                   (kAbsoluteFullTurnDegrees - kAbsoluteSlowTurnDegrees),
                               1.0f);
        fullTurnPower = kAbsoluteSlowTurnPower +
                        (kDriveTurnMax - kAbsoluteSlowTurnPower) * ramp;
      }
      const int turnPower = static_cast<int>(fullTurnPower * abs(driveDuty) / 255.0f);
      const float forwardMix = max(0.0f, 1.0f - errorSize / kAbsoluteForwardBlendDegrees);
      steering = headingError > 0.0f ? turnPower : -turnPower;
      throttle = static_cast<int>(driveDuty * forwardMix);
      leftCommand = throttle + steering;
      rightCommand = throttle - steering;
    }
  }

  const int largest = max(abs(leftCommand), abs(rightCommand));
  if (largest > 255) {
    leftCommand = leftCommand * 255 / largest;
    rightCommand = rightCommand * 255 / largest;
  }

  const bool braking = millis() < brakeUntilMs && !driverInput;
  if (braking) {
    brakeAllMotors();
  } else {
    for (int i = 0; i < 4; ++i) {
      driveMotor(i, kMotorOnLeft[i] ? leftCommand : rightCommand);
    }
  }

  static uint32_t lastPrintMs = 0;
  if (debugMode && millis() - lastPrintMs >= 100) {
    lastPrintMs = millis();
    Serial.printf("MODE=%s%s | LX=%4d LY=%4d RX=%4d LT=%4d RT=%4d duty=%4d | target=%6.1f heading=%6.1f error=%6.1f | left=%4d right=%4d\n",
                  driveMode == DriveMode::Manual ? "MANUAL" : "ABSOLUTE",
                  returnToZeroActive ? "+AUTO_ZERO" : "",
                  rawLeftX, rawLeftY, rawRightX, rawLeftTrigger, rawRightTrigger, driveDuty,
                  targetHeading, relativeHeading, headingError,
                  leftCommand, rightCommand);
  }

  static uint32_t lastBlePrintMs = 0;
  if (debugMode && millis() - lastBlePrintMs >= 200) {
    lastBlePrintMs = millis();
    char line[192];
    snprintf(line, sizeof(line),
             "%s%s LX=%d LY=%d RX=%d RY=%d LT=%d RT=%d BTN=%u DP=%u HDG=%.1f TGT=%.1f L=%d R=%d BRK=%d\n",
             driveMode == DriveMode::Manual ? "MAN" : "ABS",
             returnToZeroActive ? "+ZERO" : "",
             rawLeftX, rawLeftY, rawRightX, rawRightY, rawLeftTrigger, rawRightTrigger,
             controller->debugButtons(), controller->dpad(),
             relativeHeading, targetHeading, leftCommand, rightCommand, braking ? 1 : 0);
    bleDebugPublish(line);
  }

  delay(10);
}
