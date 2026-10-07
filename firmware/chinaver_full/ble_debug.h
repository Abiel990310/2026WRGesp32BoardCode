#pragma once

// BLE debug uses Bluepad32's BTstack, not Arduino BluetoothSerial.
void bleDebugSetup();
void bleDebugSetEnabled(bool enabled);
void bleDebugPublish(const char* line);
