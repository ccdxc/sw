#ifndef BUF_HDR_H_
#define BUF_HDR_H_
#include <stdint.h>
#include <stdlib.h>

#define MODEL_ZMQ_BUFF_SIZE         12288
#define MODEL_ZMQ_MEM_BUFF_SIZE     12288

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
    BUFF_TYPE_EOS_IGNORE_ADDR = 23,
    BUFF_TYPE_MEM_WRITE_PCIE = 24,
} buff_type_e;

typedef struct buffer_port_s_ {
    uint32_t    speed;      // port speed to configure
    uint32_t    val;        // used for reset/enable
    uint32_t    num_lanes;  // number of lanes for this port
} buffer_port_t;

typedef struct buffer_hdr_t_ {
    buff_type_e type;
    union {
        int         size;
        int         tcid;
    };
    union {
        int         port;   // also used for mac port num
        int         loopid;
    };
    int         cos;
    int         status;
    uint32_t    slowfast;
    uint32_t    ctime;
    uint64_t    addr;
    uint8_t     data[0];    // custom data
} buffer_hdr_t;

using namespace std;
static inline char *
model_utils_get_zmqsockstr ()
{
    static char zmqsockstr[600];
    const char  *user_str = getenv("ZMQ_SOC_DIR");
    char        *model_socket_name = NULL;
    char        *model_server_ip = NULL;

    if (getenv("MODEL_ZMQ_TYPE_TCP")) {
        model_socket_name = getenv("MODEL_ZMQ_TCP_PORT");
        if (model_socket_name == NULL) {
            model_socket_name = (char *) "50055";
        }
        model_server_ip = getenv("MODEL_ZMQ_SERVER_IP");
        if (model_server_ip == NULL) {
            model_server_ip = (char *) "0.0.0.0";
        }
        snprintf(zmqsockstr, 600, "tcp://%s:%s",
                 model_server_ip, model_socket_name);
    } else {
        model_socket_name = getenv("MODEL_SOCKET_NAME");
        if (model_socket_name == NULL) {
            model_socket_name = (char *)"zmqsock";
        }
        snprintf(zmqsockstr, 600, "ipc:///%s/%s", user_str, model_socket_name);
    }

    printf("Model ZMQ Socket String = %s\n", zmqsockstr);
    return zmqsockstr;
}

#endif

