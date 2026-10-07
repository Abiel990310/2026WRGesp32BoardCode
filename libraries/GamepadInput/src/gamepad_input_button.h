#pragma once

#ifndef GAMEPAD_INPUT_BUTTON_H
#define GAMEPAD_INPUT_BUTTON_H

#include <stdint.h>

/**
 * @~English
 * @defgroup CommonEnums
 * @brief Common enumerations used in the project.
 * @ingroup Root
 */
/**
 * @~Chinese
 * @defgroup CommonEnums 公共枚举
 * @brief 项目中通用的枚举
 * @ingroup Root
 */
namespace gamepad::input {

/**
 * @~English
 * @enum Button
 * @ingroup CommonEnums
 * @brief Button identifiers for a generic gamepad.
 */
/**
 * @~Chinese
 * @enum Button
 * @ingroup CommonEnums
 * @brief 通用游戏手柄按键枚举
 */
enum class Button : uint32_t {
  /**
   * @~English
   * @brief Up direction
   */
  /**
   * @~Chinese
   * @brief 方向上
   */
  kUp = uint32_t{1} << 0,

  /**
   * @~English
   * @brief Down direction
   */
  /**
   * @~Chinese
   * @brief 方向下
   */
  kDown = uint32_t{1} << 1,

  /**
   * @~English
   * @brief Left direction
   */
  /**
   * @~Chinese
   * @brief 方向左
   */
  kLeft = uint32_t{1} << 2,

  /**
   * @~English
   * @brief Right direction
   */
  /**
   * @~Chinese
   * @brief 方向右
   */
  kRight = uint32_t{1} << 3,

  /**
   * @~English
   * @brief Square or X
   */
  /**
   * @~Chinese
   * @brief 方形或者X
   */
  kSquareX = uint32_t{1} << 4,

  /**
   * @~English
   * @brief Triangle or Y
   */
  /**
   * @~Chinese
   * @brief 三角形或者Y
   */
  kTriangleY = uint32_t{1} << 5,

  /**
   * @~English
   * @brief Cross or A
   */
  /**
   * @~Chinese
   * @brief 叉或者A
   */
  kCrossA = uint32_t{1} << 6,

  /**
   * @~English
   * @brief Circle or B
   */
  /**
   * @~Chinese
   * @brief 圆形或者B
   */
  kCircleB = uint32_t{1} << 7,

  /**
   * @~English
   * @brief Left shoulder button 1
   */
  /**
   * @~Chinese
   * @brief 左肩键 L1
   */
  kL1 = uint32_t{1} << 8,

  /**
   * @~English
   * @brief Left trigger button
   */
  /**
   * @~Chinese
   * @brief 左扳机 L2
   */
  kL2 = uint32_t{1} << 9,

  /**
   * @~English
   * @brief Left stick click
   */
  /**
   * @~Chinese
   * @brief 左摇杆按下 L3
   */
  kL3 = uint32_t{1} << 10,

  /**
   * @~English
   * @brief Right shoulder button 1
   */
  /**
   * @~Chinese
   * @brief 右肩键 R1
   */
  kR1 = uint32_t{1} << 11,

  /**
   * @~English
   * @brief Right trigger button
   */
  /**
   * @~Chinese
   * @brief 右扳机 R2
   */
  kR2 = uint32_t{1} << 12,

  /**
   * @~English
   * @brief Right stick click
   */
  /**
   * @~Chinese
   * @brief 右摇杆按下 R3
   */
  kR3 = uint32_t{1} << 13,

  /**
   * @~English
   * @brief Select button
   */
  /**
   * @~Chinese
   * @brief 选择键
   */
  kSelect = uint32_t{1} << 14,

  /**
   * @~English
   * @brief Start button
   */
  /**
   * @~Chinese
   * @brief 开始键
   */
  kStart = uint32_t{1} << 15,

  /**
   * @~English
   * @brief Home button
   */
  /**
   * @~Chinese
   * @brief 主页键
   */
  kHome = uint32_t{1} << 16,
};

constexpr inline Button operator|(Button a, Button b) noexcept {
  return static_cast<Button>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr inline bool operator&(uint32_t bits, Button b) noexcept {
  return (bits & static_cast<uint32_t>(b)) == static_cast<uint32_t>(b);
}
}  // namespace gamepad::input
#endif