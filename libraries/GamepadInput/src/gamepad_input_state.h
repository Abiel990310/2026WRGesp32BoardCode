#pragma once

#ifndef GAMEPAD_INPUT_STATE_H
#define GAMEPAD_INPUT_STATE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "gamepad_input_axis.h"
#include "gamepad_input_button.h"

namespace gamepad::input {
#if __cplusplus >= 201703L
constexpr inline size_t kVersionMajor = 1;
constexpr inline size_t kVersionMinor = 0;
constexpr inline size_t kVersionPatch = 0;
constexpr inline size_t kAxisCount = 4;
constexpr inline uint8_t kAxisCenter = 0x80;
#else
static constexpr size_t kVersionMajor = 1;
static constexpr size_t kVersionMinor = 0;
static constexpr size_t kVersionPatch = 0;
static constexpr size_t kAxisCount = 4;
static constexpr uint8_t kAxisCenter = 0x80;
#endif

/**
 * @~English
 * @brief Represents a snapshot of gamepad input state.
 *
 * This structure is designed to be compact and stable for serialization.
 */
/**
 * @~Chinese
 * @brief 游戏手柄输入状态快照
 *
 * 该结构体布局稳定，可用于序列化与跨模块传输。
 */
struct State {
  uint32_t buttons{0};
  uint8_t axes[kAxisCount]{kAxisCenter, kAxisCenter, kAxisCenter, kAxisCenter};

  /**
   * @~English
   * @brief Construct a State from raw memory.
   * @param data Pointer to input data
   */
  /**
   * @~Chinese
   * @brief 从原始内存构造状态
   * @param data 输入数据指针
   */
  static State FromBytes(const void* data) noexcept {
    State state;
    memcpy(&state, data, sizeof(state));
    return state;
  }

  /**
   * @~English
   * @brief Reset all inputs to neutral state.
   */
  /**
   * @~Chinese
   * @brief 重置所有输入到中立状态
   */
  void Reset() noexcept {
    buttons = 0;
    for (size_t i = 0; i < kAxisCount; ++i) {
      axes[i] = kAxisCenter;
    }
  }

  /**
   * @~English
   * @brief Check if a button is pressed.
   */
  /**
   * @~Chinese
   * @brief 检查按键是否被按下
   */
  inline bool operator[](Button button) const noexcept { return (buttons & button) != 0; }

  /**
   * @~English
   * @brief Press a button.
   */
  /**
   * @~Chinese
   * @brief 按下指定按键
   */
  inline void Set(Button button) noexcept { buttons |= static_cast<uint32_t>(button); }

  /**
   * @~English
   * @brief Release a button.
   */
  /**
   * @~Chinese
   * @brief 释放指定按键
   */
  inline void Clear(Button button) noexcept { buttons &= ~static_cast<uint32_t>(button); }

  /**
   * @~English
   * @brief Access analog axis value.
   */
  /**
   * @~Chinese
   * @brief 访问模拟轴数值
   */
  inline uint8_t& operator[](Axis axis) noexcept { return axes[static_cast<size_t>(axis)]; }

  /**
   * @~English
   * @brief Access analog axis value (const).
   */
  /**
   * @~Chinese
   * @brief 访问模拟轴数值（只读）
   */
  inline const uint8_t& operator[](Axis axis) const noexcept { return axes[static_cast<size_t>(axis)]; }
};

static_assert(sizeof(State) == 8, "State layout must be stable");
}  // namespace gamepad::input
#endif