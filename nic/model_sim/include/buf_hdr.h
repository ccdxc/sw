#ifndef BUF_HDR_H_
#define BUF_HDR_H_
#include <stdint.h>

#include <stdint.h>

typedef enum buff_type_e_ {
    BUFF_TYPE_STEP_PKT = 0,
    BUFF_TYPE_GET_NEXT_PKT = 1,
    BUFF_TYPE_REG_READ = 2,
    BUFF_TYPE_REG_WRITE = 3,
    BUFF_TYPE_MEM_READ = 4,
    BUFF_TYPE_MEM_WRITE = 5,
    BUFF_TYPE_DOORBELL = 6,
    BUFF_TYPE_STATUS = 7,
    BUFF_TYPE_HBM_DUMP = 8
} buff_type_e;

typedef struct buffer_hdr_t_ {
    buff_type_e type;
    int         size;
    int         port;
    int         cos;
    int         status;
    uint64_t    addr;
    uint8_t     data[0];
} buffer_hdr_t;

#endif

