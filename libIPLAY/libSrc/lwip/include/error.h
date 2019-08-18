#ifndef __ERROR_H
#define __ERROR_H

#ifndef MP600
#error "no MP600"
#define NO_ERR                                  0
#endif


// General errors
#define ERR_GENERAL_BASE                        0x80000000

#define ERR_NULL_OBJECT                         (1 + ERR_GENERAL_BASE)  // the object is not allocated
#define ERR_OUT_OF_MEMORY                       (2 + ERR_GENERAL_BASE)  // no enough memory to allocate object
#define ERR_TASK_STARTUP_FAILURE                (3 + ERR_GENERAL_BASE)  // the tasks don't be started
#define ERR_STATE_ERROR                         (4 + ERR_GENERAL_BASE)  // state error
#define ERR_TASK_ALREADY_START_UP               (5 + ERR_GENERAL_BASE)  // task has already start up
#define ERR_ILL_OPERATION_MODE                  (6 + ERR_GENERAL_BASE)
#define ERR_ILL_TASK_CMD                        (7 + ERR_GENERAL_BASE)  //
#define ERR_ILLEGAL_ADDRESS                     (8 + ERR_GENERAL_BASE)  // address is not valid
#define ERR_ILLEGAL_CHIP_COMMAND                (9 + ERR_GENERAL_BASE)  // The chip doesn't support the command
#define ERR_UNSUPPORT_CHIP                      (10 + ERR_GENERAL_BASE) // The chip doesn't support
#define ERR_ILL_PARAMPETER                      (11 + ERR_GENERAL_BASE)
#define ERR_TASK_EXCEPTION_OCCURS               (12 + ERR_GENERAL_BASE)
#define ERR_MODULE_NOT_EXISTED                  (13 + ERR_GENERAL_BASE)

// error codes from IDU module including display devices
#define ERR_IDU_BASE                            0x80010000

#define ERR_IDU_READY_TIMEOUT                   (1 + ERR_IDU_BASE)      // time out to wait ID_DISP_STAT_REG ready
#define ERR_IDU_OUT_OF_RANGE                    (2 + ERR_IDU_BASE)      // the main window or sub window is outside IDU buffer
#define ERR_IDU_INVALID_VALUE                   (3 + ERR_IDU_BASE)      // data doesn't meet specification
#define ERR_IDU_INVALID_FORMAT                  (4 + ERR_IDU_BASE)      // data format doesn't meet specification

#define ERR_IDU_INCORRECT_STATUS                (80 + ERR_IDU_BASE)     // module is not under correct status

#define ERR_IDU_NOT_AVALIABLE                   (127 + ERR_IDU_BASE)    // the function is not available in the version



// error codes from Graphic User Interface
#define ERR_GUI_BASE                            0x80020000

#define ERR_GUI_MD_NOT_INITIALIZED              (1 + ERR_GUI_BASE)
#define ERR_GUI_NOT_INITIAL                     (2 + ERR_GUI_BASE)
#define ERR_GUI_INVALID_POSITION                (3 + ERR_GUI_BASE)
#define ERR_GUI_INVALID_SIZE                    (4 + ERR_GUI_BASE)
#define ERR_GUI_FORMAT_NOT_SUPPORT              (5 + ERR_GUI_BASE)
#define ERR_GUI_EXCEED_BUFFER_SIZE              (6 + ERR_GUI_BASE)
#define ERR_GUI_OUT_OF_RANGE                    (7 + ERR_GUI_BASE)
#define ERR_GUI_INVAILD_PARAMETER               (8 + ERR_GUI_BASE)
#define ERR_GUI_MAIN_NOT_UPDATED                (9 + ERR_GUI_BASE)
#define ERR_GUI_SUB_NOT_UPDATED                 (10 + ERR_GUI_BASE)

// error codes from image functions
#define ERR_IMAGE_BASE                          0x80030000
//JPG decode error code
#define ERR_IMAGE_ILLEGAL_FILE                  (0x01 + ERR_IMAGE_BASE)
#define ERR_IMAGE_UNSURPPORTED                  (0x02 + ERR_IMAGE_BASE)
#define ERR_IMAGE_PARAMETER                     (0x03 + ERR_IMAGE_BASE)
#define ERR_IMAGE_MODULE_NOT_INIT               (0x04 + ERR_IMAGE_BASE)
#define ERR_IMAGE_REQ_DATA_FORMAT               (0x06 + ERR_IMAGE_BASE)
#define ERR_IMAGE_COLOR_COMPONENT               (0x07 + ERR_IMAGE_BASE)
#define ERR_IMAGE_SUBSAMPLE_RATIO               (0x08 + ERR_IMAGE_BASE)
#define ERR_IMAGE_NOT_BASELINE                  (0x09 + ERR_IMAGE_BASE)
#define ERR_IMAGE_DCHUF_INDEX                   (0x0A + ERR_IMAGE_BASE)
#define ERR_IMAGE_ACHUF_INDEX                   (0x0B + ERR_IMAGE_BASE)
#define ERR_IMAGE_INVALID_DHT                   (0x0C + ERR_IMAGE_BASE)
#define ERR_IMAGE_SOS_END                       (0x0D + ERR_IMAGE_BASE)
#define ERR_IMAGE_UNKNOWN_MARKER                (0x0E + ERR_IMAGE_BASE)
#define ERR_IMAGE_DECODE_TIMEOUT                (0x0F + ERR_IMAGE_BASE)
#define ERR_IMAGE_HEADER_NOT_ENOUGH             (0x10 + ERR_IMAGE_BASE)
#define ERR_IMAGE_DATA_PRECISION                (0x11 + ERR_IMAGE_BASE)
#define ERR_IMAGE_COLOR_COMP_INDEX              (0x12 + ERR_IMAGE_BASE)
#define ERR_IMAGE_QTABLE_INDEX                  (0x13 + ERR_IMAGE_BASE)
#define ERR_IMAGE_CONFIG_REGISTER               (0x14 + ERR_IMAGE_BASE)
#define ERR_IMAGE_IMAGE_SIZE                    (0x15 + ERR_IMAGE_BASE)

#define ERR_IMAGE_CALLBACK_FUNCTION_NOT_INIT    (0x16 + ERR_IMAGE_BASE)
#define ERR_IMAGE_FORMAT_NOT_SUPPORTED          (0x17 + ERR_IMAGE_BASE)       // for JPEG header
#define ERR_IMAGE_DATA_DRROR                    (0x18 + ERR_IMAGE_BASE)

#define ERR_IMAGE_FILE_HANDLE_ERROR             (0x19 + ERR_IMAGE_BASE)
#define ERR_IMAGE_FILE_OPEN_FAIL                (0x1A + ERR_IMAGE_BASE)
#define ERR_IMAGE_ILLEGAL_EXTENSION             (0x1B + ERR_IMAGE_BASE)
#define ERR_IMAGE_FILE_STREAM_ERROR             (0x1C + ERR_IMAGE_BASE)
#define ERR_IMAGE_FILE_READ_ERROR               (0x1D + ERR_IMAGE_BASE)
#define ERR_IMAGE_FILE_SEEK                     (0x1E + ERR_IMAGE_BASE)

#define ERR_IMAGE_RESIZE_FORMAT_UNSUPPORTED     (0x1F + ERR_IMAGE_BASE)
#define ERR_IMAGE_RESIZE_TIME_OUT               (0x20 + ERR_IMAGE_BASE)

#define ERR_IMAGE_SW_BUF_INIT_ERROR             (0x21 + ERR_IMAGE_BASE)
#define ERR_IMAGE_ZOOM_STEP                     (0x22 + ERR_IMAGE_BASE)
#define ERR_IMAGE_JPEG_ENC_TIME_OUT             (0x23 + ERR_IMAGE_BASE)
#define ERR_IMAGE_WINDOW_SIZE                   (0x24 + ERR_IMAGE_BASE)

#define ERR_IMAGE_SLIDE_SHOW_UNSUPPORTED        (0x25 + ERR_IMAGE_BASE)
#define ERR_IMAGE_NO_TRANS_EFFECT               (0x26 + ERR_IMAGE_BASE)
#define ERR_IMAGE_INVALID_EFFECT_ID             (0x27 + ERR_IMAGE_BASE)
#define ERR_IMAGE_SLIDE_SHOW_EOF                (0x28 + ERR_IMAGE_BASE)

#define ERR_IMAGE_NULL_EXIF_INFO                (0x29 + ERR_IMAGE_BASE)
#define ERR_IMAGE_NULL_THUMBNAIL_INFO           (0x2A + ERR_IMAGE_BASE)
#define ERR_IMAGE_WRONG_THUMBNAIL_DATA          (0x2B + ERR_IMAGE_BASE)

#define ERR_IMAGE_RESIZE_OUT_OF_RANGE           (0x2C + ERR_IMAGE_BASE)

//camera error code
#define ERR_CAMERA_BASE                         (ERR_IMAGE_BASE + 0x8000)
#define ERR_CAMERA_PARAMETER                    (0x01 + ERR_CAMERA_BASE)
#define ERR_CAMERA_ILL_IMAGE_SIZE               (0x02 + ERR_CAMERA_BASE)
#define ERR_CAMERA_FILE_OPEN_FAIL               (0x03 + ERR_CAMERA_BASE)
#define ERR_CAMERA_FILE_WRITE_FAIL              (0x04 + ERR_CAMERA_BASE)
#define ERR_CAMERA_ILL_IMAGE_WIDTH              (0x05 + ERR_CAMERA_BASE)
#define ERR_CAMERA_ILL_IMAGE_HEIGHT             (0x06 + ERR_CAMERA_BASE)
#define ERR_CAMERA_PHOTO_FRAME_PALLETE          (0x07 + ERR_CAMERA_BASE)
#define ERR_CAMERA_UNSUPPORT_FORMAT             (0x08 + ERR_CAMERA_BASE)
#define ERR_CAMERA_EXIF_TAG_UNSUPPORT           (0x09 + ERR_CAMERA_BASE)
#define ERR_CAMERA_JPEG_ENC_TIMEOUT             (0x0A + ERR_CAMERA_BASE)

// error codes from video
#define ERR_VIDEO_BASE                          0x80040000
#define ERR_VIDEO_INVALID_FILE                  (1 + ERR_VIDEO_BASE)
#define ERR_VIDEO_UNSUPPORTED_VIDOE             (2 + ERR_VIDEO_BASE)
#define ERR_VIDEO_VOP_HEAD_FAIL                 (3 + ERR_VIDEO_BASE)
#define ERR_VIDEO_OUT_OF_RANGE                  (4 + ERR_VIDEO_BASE)
#define ERR_VIDEO_NOT_ENOUGH_MEMORY             (5 + ERR_VIDEO_BASE)
#define ERR_VIDEO_HANDLE_NOT_VALID              (6 + ERR_VIDEO_BASE)
#define ERR_VIDEO_STORAGE_FULL                  (7 + ERR_VIDEO_BASE)
#define ERR_VIDEO_SOUND_NOT_COMP_VIDEO          (8 + ERR_VIDEO_BASE)
#define ERR_VIDEO_INVALID_PARAMETER             (9 + ERR_VIDEO_BASE)
#define ERR_VIDEO_RECORD_TIME_UP                (10 + ERR_VIDEO_BASE)
#define ERR_VIDEO_BEGINNING_OF_VISUAL_CLIP      (11 + ERR_VIDEO_BASE)
#define ERR_VIDEO_END_OF_VISUAL_CLIP            (12 + ERR_VIDEO_BASE)
#define ERR_VIDEO_VISUAL_DEC_FAIL               (13 + ERR_VIDEO_BASE)
#define ERR_VIDEO_PROGRES_NONSTABLE             (14 + ERR_VIDEO_BASE)
#define ERR_VIDEO_ILLEGAL_EXTENSION             (15 + ERR_VIDEO_BASE)
#define ERR_VIDEO_ILLEGAL_FORMAT                (16 + ERR_VIDEO_BASE)
#define ERR_VIDEO_CLIP_DATA_ERROR               (17 + ERR_VIDEO_BASE)
#define ERR_VIDEO_INVAILD_PARAMETER             (18 + ERR_VIDEO_BASE)
#define ERR_VIDEO_SOUND_DEC_FAIL                (19 + ERR_VIDEO_BASE)
#define ERR_VIDEO_UNSUPPORTED_FORWARD           (20 + ERR_VIDEO_BASE)
#define ERR_VIDEO_AUTO_NAME_FAIL                (21 + ERR_VIDEO_BASE)
#define ERR_VIDEO_DEVICE_ERROR_FUNC_REGISTER    (22 + ERR_VIDEO_BASE)
#define ERR_VIDEO_DEVICE_ERROR_FUNC_RELEASE     (23 + ERR_VIDEO_BASE)
#define ERR_VIDEO_VISUAL_FRAME_PALLETE          (24 + ERR_VIDEO_BASE)

#define ERR_VIDEO_CON_BASE                      (0x100 + ERR_VIDEO_BASE)
#define ERR_VIDEO_CON_UNSUPPORTED_VIDOE_CODEC   (1 + ERR_VIDEO_CON_BASE)
#define ERR_VIDEO_CON_INVALID_CODEC_PARAMETER   (2 + ERR_VIDEO_CON_BASE)
#define ERR_VIDEO_CON_VOL_OUT_OF_RANGE          (3 + ERR_VIDEO_CON_BASE)
#define ERR_VIDEO_CON_UNSUPPORTED_SOUND_CODEC   (4 + ERR_VIDEO_CON_BASE)
#define ERR_VIDEO_CON_ILLEGAL_VISUAL_ACTION     (5 + ERR_VIDEO_CON_BASE)
#define ERR_VIDEO_CON_STREAM_BUFFER_FULL        (6 + ERR_VIDEO_CON_BASE)
#define ERR_VIDEO_CON_MP4_ENOCDE_FAIL           (7 + ERR_VIDEO_CON_BASE)

#define ERR_VIDEO_FMT_BASE                      (0x200 + ERR_VIDEO_BASE)
// MP4 file format error code
#define ERR_VIDEO_FMT_INVAILD_PARAMETER         (1 + ERR_VIDEO_FMT_BASE)
#define ERR_VIDEO_FMT_INVALID_FILE              (2 + ERR_VIDEO_FMT_BASE)
#define ERR_VIDEO_FMT_NOT_ENOUGH_MEMORY         (3 + ERR_VIDEO_FMT_BASE)
#define ERR_VIDEO_FMT_UNSUPPORTED_VISUAL_TYPE   (4 + ERR_VIDEO_FMT_BASE)
#define ERR_VIDEO_FMT_UNSUPPORTED_SOUND_TYPE    (5 + ERR_VIDEO_FMT_BASE)

#define ERR_MP4_FMT_NO_MOOV_BOX                 (12 + ERR_VIDEO_FMT_BASE)
#define ERR_MP4_FMT_NO_MDAT_BOX                 (13 + ERR_VIDEO_FMT_BASE)
#define ERR_MP4_FMT_READ_BOX                    (14 + ERR_VIDEO_FMT_BASE)
#define ERR_MP4_FMT_INVALID_BOX                 (15 + ERR_VIDEO_FMT_BASE)
#define ERR_MP4_FMT_UNKNOWN_BOX                 (16 + ERR_VIDEO_FMT_BASE)
#define ERR_MP4_FMT_READ_DESCRIPTOR             (17 + ERR_VIDEO_FMT_BASE)
#define ERR_MP4_FMT_INVALID_DESCRIPTOR          (18 + ERR_VIDEO_FMT_BASE)


#define ERR_AVI_FMT_INVALID_CHUNK               (50 + ERR_VIDEO_FMT_BASE)
#define ERR_AVI_FMT_INVALID_STREAM              (51 + ERR_VIDEO_FMT_BASE)

// error codes from audio
#define ERR_AUDIO_BASE                          0x80050000


#define ERR_AUDIO_INVALID_FILE                  (1 + ERR_AUDIO_BASE)
#define ERR_AUDIO_UNSUPPORTED_AUDIO             (2 + ERR_AUDIO_BASE)
#define ERR_AUDIO_RECORDING_END                 (3 + ERR_AUDIO_BASE)
#define ERR_AUDIO_PLAYING_END                   (4 + ERR_AUDIO_BASE)
#define ERR_AUDIO_RECORDING_STORAGE_FULL        (5 + ERR_AUDIO_BASE)
#define ERR_AUDIO_HANDLE_NOT_VALID              (6 + ERR_AUDIO_BASE)
#define ERR_AUDIO_STORAGE_FULL                  (7 + ERR_AUDIO_BASE)
#define ERR_AUDIO_PLAYING_BITSTREAM_ERROR       (8 + ERR_AUDIO_BASE)
#define ERR_AUDIO_INVALID_PARAMETER             (9 + ERR_AUDIO_BASE)
#define ERR_AUDIO_RECORD_TIME_UP                (10 + ERR_AUDIO_BASE)
#define ERR_AUDIO_BEGINNING_OF_FILE_HEADER      (11 + ERR_AUDIO_BASE)
#define ERR_AUDIO_END_OF_AUDIO_BITSTREAM        (12 + ERR_AUDIO_BASE)
#define ERR_AUDIO_AUDIO_DEC_FAIL                (13 + ERR_AUDIO_BASE)
#define ERR_AUDIO_PROGRES_NONSTABLE             (14 + ERR_AUDIO_BASE)
#define ERR_AUDIO_ILLEGAL_EXTENSION             (15 + ERR_AUDIO_BASE)
#define ERR_AUDIO_ILLEGAL_FORMAT                (16 + ERR_AUDIO_BASE)
#define ERR_AUDIO_BITSTREAM_DATA_ERROR          (17 + ERR_AUDIO_BASE)
#define ERR_AUDIO_FILE_OPEN_ERROR               (18 + ERR_AUDIO_BASE)
#define ERR_AUDIO_UNSUPPORTED_FORAMAT           (19 + ERR_AUDIO_BASE)
#define ERR_AUDIO_INCORRECT_STATE               (20 + ERR_AUDIO_BASE)
#define ERR_AUDIO_UNSUPPORTED_FORWARD           (21 + ERR_AUDIO_BASE)
#define ERR_AUDIO_RECORD_AUTO_NAME_FAIL         (22 + ERR_AUDIO_BASE)
#define ERR_AUDIO_DEVICE_ERROR_FUNC_REGISTER    (23 + ERR_AUDIO_BASE)
#define ERR_AUDIO_DEVICE_ERROR_FUNC_RELEASE     (24 + ERR_AUDIO_BASE)
#define ERR_AUDIO_NO_SUPPORT_PSEUDO_I2S          (25 + ERR_AUDIO_BASE)

#define ERR_FM_TUNER_BASE                       (0x100 + ERR_AUDIO_BASE)
#define ERR_FM_TUNER_INVAILD_FREQUENCY          (1 + ERR_FM_TUNER_BASE)

// error codes from file system
#define ERR_FILE_BASE                           0x80060000

#define ERR_FS_DRIVE_NOT_FORMAT                 (1 + ERR_FILE_BASE)
#define ERR_FS_DRIVE_INVALID                    (2 + ERR_FILE_BASE)
#define ERR_FS_DRIVE_FORMAT_FAIL                (3 + ERR_FILE_BASE)
#define ERR_FS_STORAGE_FULL                     (4 + ERR_FILE_BASE)
#define ERR_FS_DIR_OPERATION_FAIL               (5 + ERR_FILE_BASE)
#define ERR_FS_DIR_MOVE_OUT_RANGE               (6 + ERR_FILE_BASE)
#define ERR_FS_DIR_PATH_LEN_OVERFLOW            (7 + ERR_FILE_BASE)
#define ERR_FS_DUPLICATED_SHORT_NAME            (8 + ERR_FILE_BASE)
#define ERR_FS_DUPLICATED_LONG_NAME             (9 + ERR_FILE_BASE)
#define ERR_FS_DUPLICATED_NAME                  (10 + ERR_FILE_BASE)
#define ERR_FS_NO_LONG_NAME                     (11 + ERR_FILE_BASE)
#define ERR_FS_AUTO_NAME_FAIL                   (12 + ERR_FILE_BASE)
#define ERR_FS_FILE_OPERATION_FAIL              (13 + ERR_FILE_BASE)
#define ERR_FS_FILE_HANDLE_FULL                 (14 + ERR_FILE_BASE)
#define ERR_FS_FILE_HANDLE_INVALID              (15 + ERR_FILE_BASE)
#define ERR_FS_FILE_SEEK_OUT_OF_RANGE           (16 + ERR_FILE_BASE)
#define ERR_FS_NODE_NOT_FOUND                   (17 + ERR_FILE_BASE)
#define ERR_FS_PATH_NAME_ERR                    (18 + ERR_FILE_BASE)

#define ERR_STORAGE_DEVICE_INSERT               (19 + ERR_FILE_BASE)
#define ERR_STORAGE_DEVICE_REMOVE               (20 + ERR_FILE_BASE)
#define ERR_STORAGE_DEVICE_FATAL_ERROR          (21 + ERR_FILE_BASE)
#define ERR_STORAGE_DEVICE_INVALID              (22 + ERR_FILE_BASE)

// error codes from USB
#define ERR_USB_BASE                            0x80070000
#define ERR_USB_FUNC_NOT_SUPPORTED              ERR_USB_BASE + 1;

// error codes from GPIO
#define ERR_GPIO_BASE                           0x80080000
#define ERR_GPIO_INVALID_NUMBER                 (1 + ERR_GPIO_BASE)



// error codes from Network
#define ERR_NETWORK_BASE                        0x80090000

#define ERR_NETWORK_INIT_FAIL                   (1 + ERR_FILE_BASE)

// error codes from Shell
#define ERR_SHELL_BASE                          0x800A0000

#define ERR_SHELL_GENERAL_FAIL                  (1 + ERR_SHELL_BASE)
#define ERR_SHELL_NODATA                        (2 + ERR_SHELL_BASE)

// error codes from Shell
#define ERR_TS_BASE                             0x800B0000

#define ERR_TS_GENERAL_FAIL                     (1 + ERR_TS_BASE)
#define ERR_TS_TS_NOT_EXIST                     (2 + ERR_TS_BASE)
#define ERR_TS_HWR_NOT_EXIST                    (3 + ERR_TS_BASE)
#define ERR_TS_ADJUST_FAIL                      (4 + ERR_TS_BASE)
#define ERR_TS_NODATA                           (5 + ERR_TS_BASE)
#define ERR_TS_ADJUST_TIMEOUT                   (6 + ERR_TS_BASE)

// error code for Navigation
#define ERR_NAV_BASE                            0x800C0000

#define ERR_NAV_NOT_INITIAL                     (1 + ERR_GUI_BASE)
#define ERR_NAV_FUNC_NOT_SUPPORT                (2 + ERR_GUI_BASE)
#define ERR_NAV_TASK_NOT_INITIAL                (3 + ERR_GUI_BASE)

// error code for I2C
#define ERR_I2C_BASE                            0x800D0000

#define ERR_I2C_WRITE_FAIL                      (1 + ERR_I2C_BASE)
#define ERR_I2C_READ_FAIL                       (2 + ERR_I2C_BASE)

// error code for SIU/SENSOR
#define ERR_SIU_BASE                            0x800E0000

#define ERR_SIU_WRONG_SENSOR_ID                 (1 + ERR_SIU_BASE)
#define ERR_SIU_NO_DRIVER_MOUNTED               (2 + ERR_SIU_BASE)
#define ERR_SIU_AEAWB_CONVERGE_TIMEOUT          (3 + ERR_SIU_BASE)

#endif  // __ERROR_H

