#include "io.h"

namespace io {
  OutputPin LED(GPIOC, GPIO_PIN_13, 0, 0);
  OutputPin TEST1(GPIOA, GPIO_PIN_6, 1, 0);
  OutputPin TEST2(GPIOA, GPIO_PIN_7, 1, 0);
  OutputPin TEST3(GPIOA, GPIO_PIN_8, 1, 0);

  void setup() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    LED.init();
    TEST1.init();
    TEST2.init();
    TEST3.init();
  }
}  // namespace io.