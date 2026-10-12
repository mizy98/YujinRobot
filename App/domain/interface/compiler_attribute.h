#ifndef COMPILER_ATTRIBUTE_H
#define COMPILER_ATTRIBUTE_H

#if defined(_MSC_VER) || defined(MOCK_BUILD) || defined(WIN32_BUILD)
#define APP_CCMRAM
#elif defined(STM32F407xx) || defined(USE_HAL_DRIVER)
#define APP_CCMRAM __attribute__((section(".ccmram")))
#else
#define APP_CCMRAM
#endif

#if defined(__GNUC__)
#define APP_ALIGN_8 __attribute__((aligned(8)))
#else
#define APP_ALIGN_8
#endif

#endif  // COMPILER_ATTRIBUTE_H
