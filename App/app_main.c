#include "application/service/rtos_startup.h"
#include "application/service/startup.h"
#include "infrastructure/service/platform.h"

CCMRAM App app = {0};

int main() {
    // initialize MCU peripherals by using the STM32CubeMX generator
    MX_Init();

    if (Startup_Initialize(&app) != APP_RESULT_OK) {
        // handle error
    }

    if (RtosStartup_Initialize(&app) != APP_RESULT_OK) {
        // handle error
    }

    // must be unreachable, since the RTOS scheduler has been started
    while (true) {
    }
}
