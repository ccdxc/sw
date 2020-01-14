#ifndef _PAL_TYPES_H_
#define _PAL_TYPES_H_

#define FREEACCESS 0x01
#define CONTROLLEDACCESS 0x00

enum {
    PAL_ENV_ASIC = 0,
    PAL_ENV_HAPS = 1,
    PAL_ENV_ZEBU = 2
};

typedef enum {
    MATTR_DEVICE,
    MATTR_UNCACHED,
    MATTR_CACHED
} mattr_t;

typedef enum {
    MEMLOCK = 0,
    CPLDLOCK,
    SBUSLOCK,
    LOCK_LAST // MUST BE LAST
} pal_lock_id_t;

typedef enum {
   LCK_FAIL = 0,
   LCK_SUCCESS = 1
} pal_lock_ret_t;

typedef enum {
   LED_COLOR_NONE = 0,
   LED_COLOR_GREEN = 1,
   LED_COLOR_YELLOW = 2
} pal_led_color_t;

typedef enum {
   LED_FREQUENCY_0HZ = 0,
   LED_FREQUENCY_2HZ = 1,
   LED_FREQUENCY_1HZ = 2,
   LED_FREQUENCY_05HZ = 3
} pal_led_frequency_t;

/* Keep the order as is. */
typedef enum pal_uuid_e {
    NICMGR      =       0,
    HAL         =       1,
    PCIEMGR     =       2
} pal_uuid_t;

typedef enum {
    PAL_UPGRADE_COMPLETED = 0,
    PAL_UPGRADE_ERASED_DONE,
    PAL_UPGRADE_WRITE_DONE,
    PAL_UPGRADE_VERIFY_DONE
} pal_cpld_status_t;
#endif

