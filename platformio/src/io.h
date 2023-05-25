#pragma once

// TODO: Skip the HAL and access read/write the pins directly from the
// registers.

#include <Arduino.h>

class OutputPin {
 public:
  OutputPin(GPIO_TypeDef* gpio_port, uint16_t gpio_pin, bool positive_polarity, bool initial_value)
      : _gpio_port(gpio_port), _gpio_pin(gpio_pin), _positive_polarity(positive_polarity), _initial_value(initial_value) {
  }

  void init() {
    // Set as an output.
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = _gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(_gpio_port, &GPIO_InitStruct);
    set(_initial_value);
  }

  inline void on() {
    // _gpio_port->BSRR = _gpio_pin;
    HAL_GPIO_WritePin(_gpio_port, _gpio_pin, _positive_polarity?  GPIO_PIN_SET: GPIO_PIN_RESET);
  }

  inline void off() {
    HAL_GPIO_WritePin(_gpio_port, _gpio_pin, _positive_polarity? GPIO_PIN_RESET : GPIO_PIN_SET);
  }

  inline void set(bool is_on) {
    HAL_GPIO_WritePin(_gpio_port, _gpio_pin,
                      (is_on == _positive_polarity) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }

  inline void toggle() { HAL_GPIO_TogglePin(_gpio_port, _gpio_pin); }

 private:
  GPIO_TypeDef* const _gpio_port;
  const uint16_t _gpio_pin;
  const bool _positive_polarity;
  const bool _initial_value;
};

namespace io {

void setup();

// LED is active on low.
extern OutputPin LED;

extern OutputPin TEST1;
extern OutputPin TEST2;
extern OutputPin TEST3;
}  // namespace io.