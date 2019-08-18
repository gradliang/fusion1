#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#include "os.h"
#include "taskid.h"

#ifndef NULL_POINTER
    #define NULL_POINTER                ( (void *) NULL )
#endif

// Task related definition

// Normal Task Size
#define TASK_STACK_SIZE                 0x1000                          // 4K

// Special Task Size
#define USB_TASK_STACK_SIZE             TASK_STACK_SIZE
#define FILE_IO_TASK_STACK_SIZE         2048//TASK_STACK_SIZE
#define TS_TASK_STACK_SIZE              TASK_STACK_SIZE
#define GAME_TASK_STACK_SIZE            TASK_STACK_SIZE                 //............................................................


// Task Group Priority, base on HIGHEST_TASK_PRIORITY is 38
#define ISR_TASK_MAX_PRI                (HIGHEST_TASK_PRIORITY - 1)     // 38
#define COMMAND_TASK_H_MAX_PRI          (HIGHEST_TASK_PRIORITY - 4)     // MMP, 34
//#define COMMAND_TASK_H_MAX_PRI          (HIGHEST_TASK_PRIORITY - 15)     // MMP, 15
#define MODULE_IO_TASK_MAX_PRI          (HIGHEST_TASK_PRIORITY - 8)     // 30
#define MODULE_TASK_MAX_PRI             (HIGHEST_TASK_PRIORITY - 13)    // 25
#define COMMAND_TASK_M_MAX_PRI          (HIGHEST_TASK_PRIORITY - 14)    // MMP, 34
#define UI_TASK_MAX_PRI                 (HIGHEST_TASK_PRIORITY - 15)    // PMP, 15
#define COMMAND_TASK_L_MAX_PRI          (HIGHEST_TASK_PRIORITY - 20)    // MMP, 34

// ISR
#define DEVIO_TASK_PRI                  ISR_TASK_MAX_PRI
#define SYSTEM_TASK_PRI                 ISR_TASK_MAX_PRI
#define VIDEO_ISR_TASK_PRI              ISR_TASK_MAX_PRI
//#define HIU_CMD_TASK_PRI                ISR_TASK_MAX_PRI
//#define INT_HANDLE_TASK_PRI             (ISR_TASK_MAX_PRI - 1)
#define CAMERA_ISR_TASK_PRI             (ISR_TASK_MAX_PRI - 1)

// Moudle IO
//#define FILE_IO_TASK_PRI                MODULE_IO_TASK_MAX_PRI          // Camera
#define FILE_IO_TASK_PRI                (MODULE_TASK_MAX_PRI - 2)       // Camera
//#define AUDIO_IO_TASK_PRI               MODULE_IO_TASK_MAX_PRI          // Audio
#define VIDEO_IO_TASK_PRI               MODULE_IO_TASK_MAX_PRI          // Video
#define FILESYSTEM_IO_TASK_PRI          MODULE_IO_TASK_MAX_PRI          // File Streaming

// Module
// Need high priority module
#define VIDEO_ACTION_TASK_PRI           MODULE_TASK_MAX_PRI
#define AUDIO_MIXER_TASK_PRI            MODULE_TASK_MAX_PRI
#define TS_TASK_PRI                     MODULE_TASK_MAX_PRI
#define CAMERA_TASK_PRI                 (MODULE_TASK_MAX_PRI - 1)
#define AUDIO_ACTION_TASK_PRI           (VIDEO_ACTION_TASK_PRI - 1)

// Normal priority module
#define WATCHDOG_TASK_PRI               (MODULE_TASK_MAX_PRI - 1)
#define NAV_TASK_PRI                    (MODULE_TASK_MAX_PRI - 2)
#define IMAGE_TASK_PRI                  (MODULE_TASK_MAX_PRI - 3)
#define GAME_TASK_PRI                   (MODULE_TASK_MAX_PRI - 3)


// no priority issue module
#ifdef MP600
#define NETWORK_TASK_PRI                DRIVER_PRIORITY
#else
#error "no MP600"
#define NETWORK_TASK_PRI                (MODULE_TASK_MAX_PRI - 4)
#endif
#define USB_TASK_PRI                    (MODULE_TASK_MAX_PRI - 5)
/*
#ifndef MP600
// System Software Interrupt Constants
enum {
    INT_SERVICE_BASE                    = SYSTEM_MESSAGE_INT,
    INT_SYSTEM_READY,                   // (0x01 + INT_SERVICE_BASE)
    INT_CARD_CHANGE,                    // (0x02 + INT_SERVICE_BASE)
    INT_GET_FILE_LENGTH,                // (0x03 + INT_SERVICE_BASE)
    INT_FILE_SEEK,                      // (0x04 + INT_SERVICE_BASE)
    INT_GET_FILE_POSITION,              // (0x05 + INT_SERVICE_BASE)
    INT_FILE_READ,                      // (0x06 + INT_SERVICE_BASE)
    INT_FILE_WRITE,                     // (0x07 + INT_SERVICE_BASE)
    INT_DELETE_PROGRESS,                // (0x08 + INT_SERVICE_BASE)
    INT_COPY_PROGRESS,                  // (0x09 + INT_SERVICE_BASE)
    INT_DATA_BRIDGE,                    // (0x0A + INT_SERVICE_BASE)
    INT_PIC_BRIDGE,                     // (0x0B + INT_SERVICE_BASE)
    INT_VIDEO_RECORD,                   // (0x0C + INT_SERVICE_BASE)
    INT_VIDEO_PLAY,                     // (0x0D + INT_SERVICE_BASE)
    INT_AUDIO_RECORD,                   // (0x0E + INT_SERVICE_BASE)
    INT_AUDIO_PLAY,                     // (0x0F + INT_SERVICE_BASE)
    INT_GET_LU_CAPACITY,                // (0x10 + INT_SERVICE_BASE)
    INT_LU_READ,                        // (0x11 + INT_SERVICE_BASE)
    INT_LU_WRITE,                       // (0x12 + INT_SERVICE_BASE)
    INT_OUTPUT_BASEBAND,                // (0x13 + INT_SERVICE_BASE)
    INT_JPG_ENCODE,                     // (0x14 + INT_SERVICE_BASE)
    INT_CONTINUOUS_SHOT,                // (0x15 + INT_SERVICE_BASE)
    INT_BLUETOOTH_INT,                  // (0x16 + INT_SERVICE_BASE)
    INT_MATRIX_SHOT,                    // (0x17 + INT_SERVICE_BASE)
    INT_TIMER_SNAP_SHOT,                // (0x18 + INT_SERVICE_BASE)
    INT_PAGE_DONE,                      // (0x19 + INT_SERVICE_BASE)
    INT_RTC_ALARM,                      // (0x1A + INT_SERVICE_BASE)//alarm

    INT_SERVICE_MAX
};
#endif
*/
// System Software Exception Constants
#define EXP_SERVICE_BASE                SYSTEM_MESSAGE_EXCEPTION
#define EXP_DISK_FATAL_ERR              (0x01 + EXP_SERVICE_BASE)
#define EXP_DISK_REMOVE                 (0x02 + EXP_SERVICE_BASE)
#define EXP_CMD_TIMEOUT                 (0x03 + EXP_SERVICE_BASE)

#define MODIFY_EXP_VIDEO_RECORD         1
#define MODIFY_EXP_VIDEO_PLAY           2
#define MODIFY_EXP_IMAGE                3
#define MODIFY_EXP_CAMERA               4
#define MODIFY_EXP_AUDIO                5
#define MODIFY_EXP_GRAPHIC              6
#define MODIFY_EXP_FILE                 7
#define MODIFY_EXP_USB                  8
#define MODIFY_EXP_DATA                 9


/*
 * network buffer pool
 */
//#if USB_WIFI == AR2524_WIFI
//#define MAX_NETPOOL_BUFFER_SIZE 4800
//#define MAX_NETPOOL_ALLOC_SIZE (MAX_NETPOOL_BUFFER_SIZE+32)
//#define MAX_NUM_BUFFERS (NETPOOL_BUF_SIZE/ MAX_NETPOOL_ALLOC_SIZE)
//#else
//#define MAX_NETPOOL_ALLOC_SIZE 2560
//#define MAX_NETPOOL_BUFFER_SIZE (MAX_NETPOOL_ALLOC_SIZE-32)
//#define MAX_NUM_BUFFERS 500
//#endif


#endif

