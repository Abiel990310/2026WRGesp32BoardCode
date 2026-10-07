#pragma once
#include <cstdint>

struct CompassRecovery {
  static constexpr uint32_t kRetryMs = 3000;
  uint32_t lastAttemptMs = 0;
  void attempted(uint32_t now) { lastAttemptMs = now; }
  bool retryAllowed(uint32_t now, bool controlsNeutral, bool automaticTurn, bool braking) const {
    return controlsNeutral && !automaticTurn && !braking && uint32_t(now-lastAttemptMs) >= kRetryMs;
  }
  static bool requiresManualFallback(bool absolute, bool valid, bool zeroSet) {
    return absolute && (!valid || !zeroSet);
  }
};
