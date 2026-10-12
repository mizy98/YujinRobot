// #include "task_ui_comm_usb.h"

// #include "../../domain/interface/system_time.h"
// #include "../../domain/interface/ui_comm.h"
// #include "../../domain/service/ui_protocol.h"
// #include "../service/ui_comm_handler.h"

// #define self (_task_ui_comm_usb)
// #define super (_task_ui_comm_usb.super)

// APP_CCMRAM static TaskUiComm _task_ui_comm_usb;
// APP_CCMRAM static uint8_t _stack[TASK_UI_COMM_USB_STACK_SIZE];
// static Rtos* _rtos;

// static void _runner(Rtos* rtos) {
//     switch (super.state) {
//         case TASK_STATE_RESET: {
//             super.state = TASK_STATE_RUN;
//             break;
//         }

//         case TASK_STATE_RUN: {
//             UiProtocol_Process();
//             break;
//         }

//         case TASK_STATE_ERROR: {
//             break;
//         }
//     }
// }

// static void _wrapper(void* argument) {
//     while (true) {
//         _runner((Rtos*)argument);
//     }
// }

// AppResult TaskUiComm_Initialize(Rtos* rtos) {
//     void* task = Rtos_CreateStaticTask(_wrapper, NULL, "ui_comm_usb", _stack, sizeof(_stack),
//                                        TASK_UI_COMM_PRIORITY);
//     if (task == NULL) {
//         APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_UI_COMM_CREATE);
//         return APP_RESULT_FAIL;
//     }

//     super.handle = task;
//     super.state = TASK_STATE_RESET;
//     super.error_info.code = APP_ERROR_NONE;
//     super.stack_usage = 0.0f;
//     super.stack = _stack;
//     super.stack_size = sizeof(_stack);

//     _rtos = rtos;

//     rtos->task.ui_comm = &self;

//     return APP_RESULT_OK;
// }

// AppResult TaskUiComm_Deinitialize(void) {
//     if (Rtos_DeleteTask(super.handle)) {
//         APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_UI_COMM_DELETE);
//         return APP_RESULT_FAIL;
//     }

//     if (Rtos_DeleteTask(super.handle)) {
//         APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_UI_COMM_DELETE);
//         return APP_RESULT_FAIL;
//     }

//     return APP_RESULT_OK;
// }

// void TaskUiComm_GetLastErrorInfo(AppErrorInfo* info) {
//     if (info != NULL) {
//         *info = super.error_info;
//     }
// }
