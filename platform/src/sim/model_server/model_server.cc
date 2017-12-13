/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <Python.h>
#include "scapy_pkt_gen.h"
#include "cap_env_base.h"
#include "cpu.h"
#include <iomanip>
#include <zmq.h>
#include "HBM.h"
#include "HOST_MEM.h"
#include "buf_hdr.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <signal.h>
#include "../../utils/host_mem/params.hpp"
#include "zmq_wait.h"

#define MODEL_ZMQ_BUFF_SIZE             12288
#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

cap_env_base *g_env;

/*
 * This block describes the gateway interface between
 * the model_server and the sim_server.
 */
extern "C" {
/* imported api's to model_server from sim_server */
void sim_server_init(int argc, char *argv[]);
void sim_server_shutdown(void);
int sim_server_read_clientmem(const u_int64_t addr, 
                              void *buf,
                              const size_t len);
int sim_server_write_clientmem(const u_int64_t addr,
                               const void *buf, 
                               const size_t len);
int sim_server_sync_request(void);
int sim_server_sync_release(void);

/* exported api's from model_server to sim_server */
int
model_server_step_doorbell(u_int64_t addr, u_int64_t data)
{
    g_env->step_doorbell(addr, data);
    return 0;
}

int
model_server_read_reg(u_int64_t addr, u_int32_t *data)
{
    return g_env->read_reg(addr, *data);
}

int
model_server_write_reg(u_int64_t addr, u_int32_t data)
{
    return g_env->write_reg(addr, data);
}

int
model_server_read_mem(u_int64_t addr, u_int8_t *buf, size_t size)
{
    return g_env->read_mem(addr, buf, size);
}

int
model_server_write_mem(u_int64_t addr, u_int8_t *buf, size_t size)
{
    return g_env->write_mem(addr, buf, size);
}

} /* extern "C" */

extern "C" void __gcov_flush();
namespace utils {

class HostMem : public pen_mem_base {
 public:
  HostMem() {
  }
  virtual ~HostMem() {
  }
  virtual bool burst_read(uint64_t addr, unsigned char *data,
                          unsigned int len, bool secure, bool reverse_bo) {
      addr &= ~(1ULL << 63);
      return sim_server_read_clientmem(addr, data, len) >= 0;
  }
  virtual bool burst_write(uint64_t addr, const unsigned char *data,
                           unsigned int len, bool secure, bool reverse_bo) {
      addr &= ~(1ULL << 63);
      return sim_server_write_clientmem(addr, data, len) >= 0;
  }

 private:
};

}  // namespace utils

static void dumpHBM (void) {
      auto it = HBM::access()->begin();
      auto lst = HBM::access()->end();
      while (it != lst) {
          uint64_t addr = (*it);
          std::cout << std::hex << "Addr 0x" << addr <<
                    std::hex << " data 0x" << HBM::access()->get<uint32_t>(addr)
                    << std::dec << std::endl;
          it++;
      }
      return;
}

void process_buff (buffer_hdr_t *buff, cap_env_base *env) {
    switch (buff->type) {
        case BUFF_TYPE_STEP_PKT:
        {
            std::vector<unsigned char> pkt_vector(buff->data, buff->data + buff->size);
            uint32_t port;
            uint32_t cos;
            /* Send packet through the model */
            env->step_network_pkt(pkt_vector, buff->port);
            buff->type = BUFF_TYPE_STATUS;
            buff->status = 0;
            std::cout << "step_network_pkt port: " << port << " size: " << buff->size << std::endl;
        }
            break;
        case BUFF_TYPE_GET_NEXT_PKT:
        {
            std::vector<uint8_t> out_pkt;
            uint32_t port;
            uint32_t cos;
            /* Get output packet from the model */
            if (!env->get_next_pkt(out_pkt, port, cos)) {
                //std::cout << "ERROR: no packet" << std::endl;
                buff->type = BUFF_TYPE_STATUS;
                buff->status = -1;
            } else {
                buff->size = out_pkt.size();
                buff->port = port;
                buff->cos = cos;
                memcpy(buff->data, out_pkt.data(), out_pkt.size());
                std::cout << "get_next_pkt port: " << port << " cos: " << cos << " size: " << buff->size << std::endl;
            }
        }
            break;
        case BUFF_TYPE_REG_READ:
        {
            uint32_t data;
            uint64_t addr;
            addr = buff->addr;
            bool ret = env->read_reg(addr, data);
            ret = true;
            if (!ret) {
                std::cout << "ERROR: Reading register" << std::endl;
                buff->type = BUFF_TYPE_STATUS;
                buff->status = -1;
            } else {
                buff->size = sizeof(uint32_t);
                memcpy(buff->data, &data, buff->size);
                printf("read_reg addr: 0x%lx data: 0x%x\n", addr, data);
            }
        }
            break;
        case BUFF_TYPE_REG_WRITE:
        {
            uint32_t data;
            uint64_t addr = buff->addr;
            memcpy(&data, buff->data, sizeof(uint32_t));
            bool ret = env->write_reg(addr, data);
            ret = true;
            if (!ret) {
                std::cout << "ERROR: Writing register" << std::endl;
                buff->type = BUFF_TYPE_STATUS;
                buff->status = -1;
            } else {
                buff->type = BUFF_TYPE_STATUS;
                buff->status = 0;
                printf("write_reg addr: 0x%lx  data: 0x%x\n", addr, data);
            }
        }
            break;
        case BUFF_TYPE_MEM_READ:
        {
            uint64_t addr = buff->addr;
            bool ret = env->read_mem(addr, buff->data, buff->size);
            ret = true;
            if ((buff->size > MODEL_ZMQ_BUFF_SIZE) || !ret) {
                std::cout << "ERROR: Reading memory" << std::endl;
                buff->type = BUFF_TYPE_STATUS;
                buff->status = -1;
            } else {
                printf("read_mem addr: 0x%lx size: %d\n", addr, buff->size);
            }
        }
            break;
        case BUFF_TYPE_MEM_WRITE:
        {
            uint64_t addr = buff->addr;
            bool ret = env->write_mem(addr, buff->data, buff->size);
            ret = true;
            if ((buff->size > MODEL_ZMQ_BUFF_SIZE) || !ret) {
                std::cout << "ERROR: Writing memory" << std::endl;
                buff->type = BUFF_TYPE_STATUS;
                buff->status = -1;
            } else {
                buff->type = BUFF_TYPE_STATUS;
                buff->status = 0;
                printf("write_mem addr: 0x%lx size: %d\n", addr, buff->size);
            }
        }
            break;
        case BUFF_TYPE_DOORBELL:
        {
            uint64_t data;
            uint64_t addr = buff->addr;
            memcpy(&data, buff->data, sizeof(uint64_t));
            env->step_doorbell(addr, data);
            buff->type = BUFF_TYPE_STATUS;
            buff->status = 0;
            std::cout << "step_doorbell addr: " << std::hex << addr << 
                         " data: " << std::hex << data << std::endl;
        }
            break;
        case BUFF_TYPE_HBM_DUMP:
        {
            std::cout << "*************** HBM dump START ***************" << std::endl;
            dumpHBM();
            std::cout << "*************** HBM dump END ***************" << std::endl;
        }
            break;
        case BUFF_TYPE_STATUS:
        default:
            assert(0);
            break;
    }
    return;
}

static void
zmq_model_recv(void *socket, void *arg)
{
    char recv_buff[MODEL_ZMQ_BUFF_SIZE];
    buffer_hdr_t *buff;

    zmq_recv (socket, recv_buff, MODEL_ZMQ_BUFF_SIZE, 0);
    buff = (buffer_hdr_t *) recv_buff;
    sim_server_sync_request();
    process_buff(buff, g_env);
    sim_server_sync_release();
    zmq_send (socket, recv_buff, MODEL_ZMQ_BUFF_SIZE, 0);
}

//  ZMQ Socket to talk to clients
static void *
init_zmq_model_server(void)
{
    const char *user = std::getenv("PWD");
    char zmqsockstr[200];
    snprintf(zmqsockstr, sizeof(zmqsockstr), "ipc:///%s/zmqsock", user);
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind(responder, zmqsockstr);
    assert (rc == 0);
    return responder;
}

static void wait_loop() {
    int rc;
    buffer_hdr_t *buff;
    char zmqsockstr[200];
    char recv_buff[MODEL_ZMQ_BUFF_SIZE];
    
    const char* user_str = std::getenv("PWD");
    snprintf(zmqsockstr, 200, "ipc:///%s/zmqsock", user_str);
    //  ZMQ Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    rc = zmq_bind(responder, zmqsockstr);
    assert (rc == 0);
    std::cout << "Model initialized! Waiting for pkts/command...." << std::endl;
    while (1) {
        rc = zmq_recv (responder, recv_buff, MODEL_ZMQ_BUFF_SIZE, 0);
        buff = (buffer_hdr_t *) recv_buff;
        process_buff(buff, g_env);
        rc = zmq_send (responder, recv_buff, MODEL_ZMQ_BUFF_SIZE, 0);
    }
    return;
}

static void
model_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    printf("Rcvd signal %u\n", sig);

    switch (sig) {
    case SIGKILL:
    case SIGINT:
        std::cout << "Rcvd SIGKILL/SIGINT, flushing code coverage data ..." << std::endl;
        printf("Rcvd SIGKILL/SIGINT, flushing code coverage data ...\n");
        fflush(stdout);
        HAL_GCOV_FLUSH();
        sim_server_shutdown();
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        std::cout << "Rcvd SIGUSR1/SIGUSR2, flushing code coverage data ..." << std::endl;
        printf("Rcvd SIGUSR1/SIGUSR2, flushing code coverage data ...\n");
        fflush(stdout);
        HAL_GCOV_FLUSH();
        wait_loop();
        break;

    default:
        printf("Not handling signal\n");
        break;
    }
    return;
}

static void
model_sig_init (void)
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = model_sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGKILL, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    return;
}

int
main(int argc, char *argv[])
{
    model_sig_init();

    utils::HostMem host_mem;
    HOST_MEM::access(&host_mem);

    sknobs_init(argc, argv);
    auto env = new cap_env_base(0);
    env->init();
    env->load_debug();
    g_env = env;

    void *responder = init_zmq_model_server();
    zmq_wait_add(responder, zmq_model_recv, NULL);

    sim_server_init(argc, argv);

    std::cout << "Model initialized! Waiting for pkts/command...." << std::endl;
    zmq_wait_loop();

    return 0;
}
