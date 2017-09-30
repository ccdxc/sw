#include <python2.7/Python.h>
#include "nic/model_sim/include/scapy_pkt_gen.h"
#include "nic/model_sim/include/cap_env_base.h"
#include "nic/model_sim/include/cpu.h"
#include <iomanip>
#include <zmq.h>
#include "nic/model_sim/include/HBM.h"
#include "nic/model_sim/include/HOST_MEM.h"
#include "nic/model_sim/include/buf_hdr.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <queue>
#include <signal.h>
#include "nic/utils/host_mem/params.hpp"

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

cap_env_base *g_env;
std::queue<std::vector<uint8_t>> g_cpu_pkts;
extern "C" void __gcov_flush();

void *context;
void *rsock;    /* All Receives: input packets, reads and writes. Blocking
                 * responses like read_mem/reg are also sent back on socket */
void *esock;    /* Send events: output packets, interrupt vector */

namespace utils {

class HostMem : public pen_mem_base {
 public:
  HostMem() {
    shmid_ = shmget(HostMemHandle(), kShmSize, IPC_CREAT | 0666);
    assert(shmid_ >= 0);
    shmaddr_ = shmat(shmid_, nullptr, 0);
    assert(shmaddr_ != (void*)-1);
  }
  virtual ~HostMem() {
    if ((shmaddr_) && (shmaddr_ != (void *)-1))
      shmdt(shmaddr_);
    shmctl(shmid_, IPC_RMID, NULL);
  }
  virtual bool burst_read(uint64_t addr, unsigned char *data,
                          unsigned int len, bool secure, bool reverse_bo) {
    addr &= (1ULL << 52) - 1;
    if ((addr >= kShmSize) || ((addr + len) > kShmSize))
      return false;
    void *sa = (void *)((uint64_t)shmaddr_ + addr);
    bcopy(sa, data, len);
    return true;
  }
  virtual bool burst_write(uint64_t addr, const unsigned char *data,
                           unsigned int len, bool secure, bool reverse_bo) {
    addr &= (1ULL << 52) - 1;
    if ((addr >= kShmSize) || ((addr + len) > kShmSize))
      return false;
    void *da = (void *)((uint64_t)shmaddr_ + addr);
    bcopy(data, da, len);
    return true;
  }

 private:
  int shmid_;
  void *shmaddr_;
};

}  // namespace utils

utils::HostMem g_host_mem;

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

void process_buff (buffer_hdr_t *buff, cap_env_base *env, zmq_msg_t *id) {
    int rc;
    switch (buff->type) {
        case BUFF_TYPE_STEP_TIMER_WHEEL:
        {
            /* Call step timer wheel update in model */
            env->step_tmr_wheel_update(buff->slowfast, buff->ctime);
            buff->type = BUFF_TYPE_STATUS;
            buff->status = 0;
    	    std::cout << "step_tmr_wheel_update slowfast: " << buff->slowfast << " ctime: " << buff->ctime << std::endl;
        }
            break;
        case BUFF_TYPE_STEP_PKT:
        {
            std::vector<unsigned char> pkt_vector(buff->data, buff->data + buff->size);
            std::vector<uint8_t> out_pkt;
            uint32_t port;
            uint32_t cos;
            /* Send packet through the model */
            env->step_network_pkt(pkt_vector, buff->port);
    	    std::cout << "step_network_pkt port: " << port << " size: " << buff->size << std::endl;
            /* Since the packets are sent to the model server by the client
             * asynchronosly in a non blocking way, we need to check for the
             * returned packets by the model and send it back to the client */
            while (env->get_next_pkt(out_pkt, port, cos)) {
                char sbuff[MODEL_ZMQ_BUFF_SIZE];
                buffer_hdr_t *send_buff = (buffer_hdr_t *) sbuff;
                
                send_buff->size = out_pkt.size();
                send_buff->port = port;
                send_buff->cos = cos;
                send_buff->type = BUFF_TYPE_GET_NEXT_PKT;
                
                memcpy(send_buff->data, out_pkt.data(), out_pkt.size());
    	        std::cout << "Got packet: get_next_pkt port: " << port << " cos: " << cos << " size: " << out_pkt.size() << std::endl;
                /* Send the buffer back to the client, on a different socket */
                rc = zmq_send (esock, send_buff, MODEL_ZMQ_BUFF_SIZE, 0);
                assert(rc != -1);
            }
    	    std::cout << "No more packets: Sending status back" << std::endl;
            char sbuff[MODEL_ZMQ_BUFF_SIZE];
            buffer_hdr_t *send_buff = (buffer_hdr_t *) sbuff;
            /* All packets are done. Send a completion event to client */
            send_buff->type = BUFF_TYPE_STATUS;
            send_buff->status = 0;
            send_buff->size = 0;
            send_buff->port = -1;
            rc = zmq_send (esock, send_buff, MODEL_ZMQ_BUFF_SIZE, 0);
            assert(rc != -1);
        }
            break;
        case BUFF_TYPE_GET_NEXT_PKT:
            /* This should not be called in non-blocking mode */
            assert(0);
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
                char sbuff[MODEL_ZMQ_BUFF_SIZE];
                buffer_hdr_t *send_buff = (buffer_hdr_t *) sbuff;
                zmq_msg_t send_id;
                zmq_msg_init(&send_id);

                send_buff->status = buff->status;
                send_buff->size = sizeof(uint32_t);
                memcpy(send_buff->data, &data, send_buff->size);

                /* For read requests, client will be blocking on the call */
                zmq_msg_copy(&send_id, id);
                rc = zmq_msg_send (&send_id, rsock, ZMQ_SNDMORE);
                assert(rc != -1);
                rc = zmq_send (rsock, send_buff, MODEL_ZMQ_BUFF_SIZE, 0);
                assert(rc != -1);
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
            }
        }
            break;
        case BUFF_TYPE_MEM_READ:
        {
            uint64_t addr = buff->addr;
            bool ret = env->read_mem(addr, buff->data, buff->size);
            ret = true;
            if ((buff->size > MODEL_ZMQ_MEM_BUFF_SIZE) || !ret) {
    	        std::cout << "ERROR: Reading memory" << std::endl;
                buff->type = BUFF_TYPE_STATUS;
                buff->status = -1;
            } else {
                char sbuff[MODEL_ZMQ_MEM_BUFF_SIZE];
                buffer_hdr_t *send_buff = (buffer_hdr_t *) sbuff;
                zmq_msg_t send_id;
                zmq_msg_init(&send_id);

                send_buff->status = buff->status;
                send_buff->size = buff->size;
                memcpy(send_buff->data, buff->data, buff->size);

                /* For read requests, client will be blocking on the call */
                zmq_msg_copy(&send_id, id);
                rc = zmq_msg_send (&send_id, rsock, ZMQ_SNDMORE);
                assert(rc != -1);
                rc = zmq_send (rsock, send_buff, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
                assert(rc != -1);
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
        case BUFF_TYPE_STEP_CPU_PKT:
        {
            char sbuff1[MODEL_ZMQ_BUFF_SIZE];
            char sbuff2[MODEL_ZMQ_BUFF_SIZE];
            buffer_hdr_t *send_buff1 = (buffer_hdr_t *) sbuff1;
            buffer_hdr_t *send_buff2 = (buffer_hdr_t *) sbuff2;
            /* Cpu packet can be sent back to the client immediately on
             * the event sock */
    	    std::cout << "step_cpu_pkt size: " << buff->size << std::endl;
            send_buff1->type = BUFF_TYPE_GET_NEXT_CPU_PKT;
            send_buff1->size = buff->size;
            send_buff1->port = 128;
            memcpy(send_buff1->data, buff->data, buff->size);
            rc = zmq_send (esock, send_buff1, MODEL_ZMQ_BUFF_SIZE, 0);
            assert(rc != -1);
            /* Send a completion event to client */
            send_buff2->type = BUFF_TYPE_STATUS;
            send_buff2->status = 0;
            send_buff2->size = 0;
            send_buff2->port = -1;
            rc = zmq_send (esock, send_buff2, MODEL_ZMQ_BUFF_SIZE, 0);
            assert(rc != -1);
        }
            break;
        case BUFF_TYPE_GET_NEXT_CPU_PKT:
            assert(0);
            break;
        case BUFF_TYPE_STATUS:
        default:
            assert(0);
            break;
    }
    return;
}

static void wait_loop() {
    int rc;
    buffer_hdr_t *buff;
    zmq_msg_t id;
    char rbuffer[MODEL_ZMQ_MEM_BUFF_SIZE] = {0};
    
    std::cout << "Non-blocking Model initialized! Waiting for pkts/command...." << std::endl;
    while (1) {
        /* Router socket: Get both the message parts */
        zmq_msg_init(&id);
        rc = zmq_msg_recv (&id, rsock, 0);
        assert(rc != -1);
        rc = zmq_recv (rsock, rbuffer, MODEL_ZMQ_MEM_BUFF_SIZE, 0);
        assert(rc != -1);
        
        buff = (buffer_hdr_t *) rbuffer;
        process_buff(buff, g_env, &id);
        zmq_msg_close(&id);
    }
    return;
}

static void
model_sig_handler (int sig)
{
    printf("Rcvd signal %u\n", sig);

    switch (sig) {
    case SIGKILL:
    case SIGINT:
        std::cout << "Rcvd SIGKILL/SIGINT, flushing code coverage data ..." << std::endl;
        fflush(stdout);
        HAL_GCOV_FLUSH();
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        std::cout << "Rcvd SIGUSR1/SIGUSR2, flushing code coverage data ..." << std::endl;
        fflush(stdout);
        HAL_GCOV_FLUSH();
        wait_loop();
        break;

    default:
        exit(0);
        break;
    }
    return;
}

static void
model_sig_init (void)
{
    struct sigaction action;
    action.sa_handler = model_sig_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
    sigaction (SIGHUP, &action, NULL);
    sigaction (SIGUSR1, &action, NULL);
    sigaction (SIGUSR2, &action, NULL);
}

int main (int argc, char ** argv)
{
    int rc;

    model_sig_init();

    HOST_MEM::access(&g_host_mem);

    sknobs_init(argc, argv);
    auto env = new cap_env_base(0);
    env->init();
    env->load_debug();
    g_env = env;

    char zmqsockstr[200];
    const char* user_str = std::getenv("PWD");
    
    //  ZMQ sockets to talk to clients
    context = zmq_ctx_new ();
    rsock = zmq_socket (context, ZMQ_ROUTER);
    esock = zmq_socket (context, ZMQ_PUB);
    
    /* Set watermark - 10mil messages */
    int wmark = 10000000;
    zmq_setsockopt (esock, ZMQ_SNDHWM, &wmark, sizeof(int));
    zmq_setsockopt (rsock, ZMQ_RCVHWM, &wmark, sizeof(int));
    zmq_setsockopt (rsock, ZMQ_SNDHWM, &wmark, sizeof(int));

    snprintf(zmqsockstr, 200, "ipc:///%s/zmqsock1", user_str);
    rc = zmq_bind(rsock, zmqsockstr);
    assert (rc == 0);

    snprintf(zmqsockstr, 200, "ipc:///%s/zmqsock2", user_str);
    rc = zmq_bind(esock, zmqsockstr);
    assert (rc == 0);

    wait_loop();
    return 0;
}
