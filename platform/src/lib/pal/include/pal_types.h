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
    LOCK_LAST // MUST BE LAST
} pal_lock_id_t;

typedef enum {
   LCK_FAIL = 0,
   LCK_SUCCESS = 1
} pal_lock_ret_t;

typedef enum {
   QSFP_LED_COLOR_NONE = 0,
   QSFP_LED_COLOR_GREEN = 1,
   QSFP_LED_COLOR_YELLOW = 2
} pal_qsfp_led_color_t;


/* Keep the order as is. */
typedef enum uuid_e {
    NICMGR      =       0,
    HAL         =       1,
    PCIEMGR     =       2
} uuid_t;

#endif

