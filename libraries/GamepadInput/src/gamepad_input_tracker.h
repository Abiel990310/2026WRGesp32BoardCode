
#pragma once

#ifndef GAMEPAD_INPUT_TRACKER_H
#define GAMEPAD_INPUT_TRACKER_H

#include "gamepad_input_state.h"

namespace gamepad::input {

/**
 * @~English
 * @brief Tracks changes in gamepad input between frames.
 *
 * Useful for detecting button press, release, hold,
 * and significant axis movement.
 */
/**
 * @~Chinese
 * @brief 手柄输入帧间状态追踪器
 *
 * 用于检测按键按下、弹起、按住以及轴变化。
 */
class Tracker {
 public:
  /**
   * @~English
   * @brief Get the current input state.
   */
  /**
   * @~Chinese
   * @brief 获取当前输入状态
   */
  const State& raw() const noexcept { return current_; }

  /**
   * @~English
   * @brief Reset tracker state to neutral.
   */
  /**
   * @~Chinese
   * @brief 重置追踪器状态
   */
  void Reset() noexcept {
    current_.Reset();
    previous_.Reset();
  }

  /**
   * @~English
   * @brief Advance to next frame (swap states).
   */
  /**
   * @~Chinese
   * @brief 推进到下一帧（交换状态）
   */
  void Tick() noexcept { previous_ = current_; }

  /**
   * @~English
   * @brief Update current frame input state.
   */
  /**
   * @~Chinese
   * @brief 更新当前帧输入状态
   */
  void Update(State state) noexcept { current_ = state; };

  /**
   * @~English
   * @brief Detect button press (falling edge).
   * @return True if button is pressed
   */
  /**
   * @~Chinese
   * @brief 检测按键按下（下降沿）
   * @return 如果按键被按下
   */
  bool pressed(Button button) const noexcept { return current_[button] && !previous_[button]; }

  /**
   * @~English
   * @brief Detect button release (rising edge).
   * @return True if button is released
   */
  /**
   * @~Chinese
   * @brief 检测按键释放（上升沿）
   * @return 如果按键被释放
   */
  bool released(Button button) const noexcept { return !current_[button] && previous_[button]; }

  /**
   * @~English
   * @brief Detect button hold.
   * @return True if button is held
   */
  /**
   * @~Chinese
   * @brief 检测按键持续
   * @return 如果按键持续
   */
  bool holding(Button button) const noexcept { return current_[button] && previous_[button]; }

  /**
   * @~English
   * @brief Detect axis movement.
   * @param threshold Threshold for detecting axis movement
   * @return True if axis has moved
   */
  /**
   * @~Chinese
   * @brief 检测轴移动
   * @param threshold 用于检测轴移动的阈值
   * @return 如果轴移动
   */
  bool AxisChanged(Axis axis, uint8_t threshold) const noexcept {
    return AxisChanged(previous_[axis], current_[axis], threshold);
  }

  /**
   * @~English
   * @brief Check if a button is pressed.
   * @return True if button is pressed
   */
  /**
   * @~Chinese
   * @brief 检查按键是否被按下
   * @return 如果按键被按下
   */
  bool operator[](Button button) const noexcept { return current_[button]; }

  /**
   * @~English
   * @brief Get axis value.
   * @return Axis value
   */
  /**
   * @~Chinese
   * @brief 获取轴值
   * @return 轴值
   */
  uint8_t operator[](Axis axis) const noexcept { return current_[axis]; }

 private:
  static bool AxisChanged(uint8_t previous, uint8_t current, uint8_t threshold) noexcept {
    if (previous == current) {
      return false;
    }

    if (current == 0 || current == 255) {
      return true;
    }

    const uint8_t diff = current > previous ? current - previous : previous - current;
    return diff >= threshold;
  }

  State current_{};
  State previous_{};
};
}  // namespace gamepad::input

#endif