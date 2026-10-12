#include "startup.h"

#include <stdint.h>
#include <string.h>

#include "../../domain/interface/system.h"

#if defined(STM32F407xx) || defined(USE_HAL_DRIVER)
extern uint32_t _sccmram;
extern uint32_t _eccmram;
#endif

AppResult Startup_Initialize(App* app) {
#if defined(STM32F407xx) || defined(USE_HAL_DRIVER)
    // CCM SRAM keeps its contents across a core reset and is not covered by the
    // startup code's data/bss init, so statics placed there (via CCMRAM/APP_CCMRAM)
    // are not guaranteed to start zeroed. Clear the whole region up front instead
    // of relying on per-variable `= {0}` initializers.
    memset(&_sccmram, 0, (size_t)((uintptr_t)&_eccmram - (uintptr_t)&_sccmram));
#endif

    memset(app, 0, sizeof(App));

    app->info = BmsInfo_CreateDefault();
    app->config = BmsConfig_CreateDefault();

    app->info.serial_number[0] = System_GetUniqueId(0);
    app->info.serial_number[1] = System_GetUniqueId(1);
    app->info.serial_number[2] = System_GetUniqueId(2);

    System_GetResetReason(&app->boot_info.reset_reason, &app->boot_info.reset_flag);

    app->boot_info.is_backup_valid = 0xFF;  // Unknown until checked by storage task

    return APP_RESULT_OK;
}
