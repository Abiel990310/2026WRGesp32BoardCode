#pragma once

#ifndef GAMEPAD_INPUT_AXIS_H
#define GAMEPAD_INPUT_AXIS_H

#include <stddef.h>
#include <stdint.h>

namespace gamepad::input {

/**
 * @~English
 * @enum Axis
 * @ingroup CommonEnums
 * @brief Axis
 */
/**
 * @~Chinese
 * @enum Axis
 * @ingroup CommonEnums
 * @brief 轴
 */
enum class Axis : size_t {
  /**
   * @~English
   * @brief Left stick X axis
   */
  /**
   * @~Chinese
   * @brief 左摇杆X轴
   */
  kLeftStickX,

  /**
   * @~English
   * @brief Left stick Y axis
   */
  /**
   * @~Chinese
   * @brief 左摇杆Y轴
   */
  kLeftStickY,

  /**
   * @~English
   * @brief Right stick X axis
   */
  /**
   * @~Chinese
   * @brief 右摇杆X轴
   */
  kRightStickX,

  /**
   * @~English
   * @brief Right stick Y axis
   */
  /**
   * @~Chinese
   * @brief 右摇杆Y轴
   */
  kRightStickY,
};
}  // namespace gamepad::input
#endif