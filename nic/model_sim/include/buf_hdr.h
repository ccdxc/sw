#ifndef BUF_HDR_H_
#define BUF_HDR_H_
#include <stdint.h>

#include <stdint.h>

#define MODEL_ZMQ_BUFF_SIZE     12288
#define MODEL_ZMQ_MEM_BUFF_SIZE 12288

typedef enum buff_type_e_ {
    BUFF_TYPE_STEP_PKT = 0,
    BUFF_TYPE_GET_NEXT_PKT = 1,
    BUFF_TYPE_REG_READ = 2,
    BUFF_TYPE_REG_WRITE = 3,
    BUFF_TYPE_MEM_READ = 4,
    BUFF_TYPE_MEM_WRITE = 5,
    BUFF_TYPE_DOORBELL = 6,
    BUFF_TYPE_STATUS = 7,
    BUFF_TYPE_HBM_DUMP = 8,
    BUFF_TYPE_STEP_CPU_PKT = 9,
    BUFF_TYPE_GET_NEXT_CPU_PKT = 10,
    BUFF_TYPE_STEP_TIMER_WHEEL = 11,
    BUFF_TYPE_MAC_CFG = 12,
    BUFF_TYPE_MAC_EN = 13,
    BUFF_TYPE_MAC_SOFT_RESET = 14,
    BUFF_TYPE_MAC_STATS_RESET = 15,
    BUFF_TYPE_MAC_INTR_EN = 16,
    BUFF_TYPE_MAC_INTR_CLR = 17,
    BUFF_TYPE_REGISTER_MEM_ADDR = 18,
    BUFF_TYPE_EXIT_SIM = 19,
    BUFF_TYPE_CONFIG_DONE = 20,
    BUFF_TYPE_TESTCASE_BEGIN = 21,
    BUFF_TYPE_TESTCASE_END = 22,
} buff_type_e;

typedef struct buffer_port_s_ {
    uint32_t    speed;      // port speed to configure
    uint32_t    val;        // used for reset/enable
    uint32_t    num_lanes;  // number of lanes for this port
} buffer_port_t;

typedef struct buffer_hdr_t_ {
    buff_type_e type;
    int         size;
    int         port;   // also used for mac port num
    int         cos;
    int         status;
    uint32_t    slowfast;
    uint32_t    ctime;
    uint64_t    addr;
    uint8_t     data[0];    // custom data
} buffer_hdr_t;

#endif

