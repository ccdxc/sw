#include <iostream>
#include <deque>
#include <thread>
#include <cmath>
#include <condition_variable>

#include <grpc++/grpc++.h>
#include <zmq.h>

#include <pthread.h>

//#include <nic/gen/proto/hal/interface.grpc.pb.h>
#include <nic/gen/proto/hal/rdma.grpc.pb.h>

#include "nic/model_sim/include/buf_hdr.h"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/utils/host_mem/host_mem.hpp"

#define Q_TYPE_RXQ     0
#define Q_TYPE_TXQ     1
#define Q_TYPE_ADMINQ  2
#define Q_TYPE_RDMA_SQ 3
#define Q_TYPE_RDMA_RQ 4
#define Q_TYPE_RDMA_CQ 5
#define Q_TYPE_RDMA_EQ 6

static int lif_base;

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef u_int64_t dma_addr_t;
#define __packed __attribute__((packed))
#define BIT(n)  (1 << (n))

#include "drivers/linux/eth/ionic/ionic_if.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace grpc;
//using namespace intf;
using namespace rdma;
using namespace std;

//std::mutex cout_mu;

typedef struct hal_req_resp_ {
    struct admin_cmd   cmd;
    struct admin_comp  comp;
    uint64_t          *pt_table;
    unsigned char              *header;
    int                header_template_size;
    u_int32_t         *done;
}hal_req_resp_t;

class HalReqBuffer {
public:
    void add(hal_req_resp_t item) {
        while (true) {
            std::unique_lock<std::mutex> locker(mu);
            cond.wait(locker, [this](){return buffer_.size() < size_;});
            buffer_.push_back(item);
            locker.unlock();
            cond.notify_all();
            return;
        }
    }
    hal_req_resp_t remove() {
        while (true)
        {
            std::unique_lock<std::mutex> locker(mu);
            cond.wait(locker, [this](){return buffer_.size() > 0;});
            hal_req_resp_t back = buffer_.front();
            buffer_.pop_front();
            locker.unlock();
            cond.notify_all();
            return back;
        }
    }
    HalReqBuffer() {}
private:
    std::mutex mu;
    std::condition_variable cond;

    std::deque<hal_req_resp_t> buffer_;
    const unsigned int size_ = 1024;
};


#if 0
unique_ptr<Interface::Stub> int_svc = 0;
unique_ptr<Interface::Stub> GetIntStub (void)
{
    char *grpc_port_env;
    std::string grpc_ep = "localhost:";
    if ((grpc_port_env = getenv("HAL_GRPC_PORT")) != NULL) {
        grpc_ep.append(grpc_port_env);
    } else {
        grpc_ep.append("50054");
    }
    shared_ptr<Channel> channel = CreateChannel(grpc_ep, InsecureChannelCredentials());
    StubOptions options;

    return Interface::NewStub(channel, options);

}
#endif

shared_ptr<Rdma::Stub> rdma_svc = 0;
shared_ptr<Rdma::Stub> GetRdmaStub (void)
{
    char *grpc_port_env;
    std::string grpc_ep = "localhost:";

    if (rdma_svc)
        return rdma_svc;
    
    if ((grpc_port_env = getenv("HAL_GRPC_PORT")) != NULL) {
        grpc_ep.append(grpc_port_env);
    } else {
        grpc_ep.append("50054");
    }
    shared_ptr<Channel> channel = CreateChannel(grpc_ep, InsecureChannelCredentials());
    StubOptions options;

    rdma_svc = Rdma::NewStub(channel, options);

    return rdma_svc;
}

#if 0
uint64_t
get_qstate_addr(uint64_t lif, uint32_t qtype, uint32_t qid) {

    //cout << "Lif = " << lif << " Qtype = " << qtype << " Qid = " << qid << endl;

    unique_ptr<Interface::Stub> int_svc = GetIntStub();
    
    ClientContext context;
    GetQStateRequestMsg request;
    GetQStateResponseMsg response;

    QStateGetReq *qstate_request = request.add_reqs();
    qstate_request->set_lif_handle(lif);
    qstate_request->set_type_num(qtype);
    qstate_request->set_qid(qid);

    Status status = int_svc->LifGetQState(&context, request, &response);
    if (status.ok()) {
        for (int i = 0; i < response.resps().size(); i++) {
            QStateGetResp qstate_response = response.resps(i);
            if (qstate_response.error_code()) {
                cout << "Error Code = " << qstate_response.error_code() << endl;
                return (0);
            } else {
                return qstate_response.q_addr();
            }
        }
    } else {
        cout << status.error_code() << ": " << status.error_message() << endl;
        return (0);
    }

    return (0);
}
#endif

#define AC_LOCAL_WRITE       0x1
#define AC_REMOTE_WRITE      0x2
#define AC_REMOTE_READ       0x4
#define AC_REMOTE_ATOMIC     0x8

extern "C" {
int simdev_read_host_mem(u_int64_t addr, void *buf, size_t size);
int simdev_write_host_mem(u_int64_t addr, void *buf, size_t size);
} /* extern "c" */

void hal_create_eq (struct create_eq_cmd  *cmd,
                    struct admin_comp     *comp,
                    hal_req_resp_t        *item)
{
    shared_ptr<Rdma::Stub> rdma_svc = GetRdmaStub();

    ClientContext context;
    RdmaEqRequestMsg request;
    RdmaEqResponseMsg response;

    RdmaEqSpec *spec = request.add_request();
    spec->set_eq_id(cmd->intr);
    spec->set_hw_lif_id(cmd->lif_id+lif_base);
    spec->set_num_eq_wqes(1u << cmd->log_depth);
    spec->set_eq_wqe_size(1u << cmd->log_stride);
    spec->set_eqe_base_addr_phy(cmd->dma_addr);
    spec->set_int_num(cmd->intr);

    Status status = rdma_svc->RdmaEqCreate(&context, request, &response);
    if (status.ok()) {
        //This loop gets executed only once.
        for (int i = 0; i < response.response().size(); i++) {
            RdmaEqResponse eq_response = response.response(i);
            comp->status = eq_response.api_status();
            cout << "lib_driver.cc: hal_create_eq RPC: SUCCESS" << endl;
        }
    } else {
        comp->status = status.error_code();
        cout << "lib_driver.cc: hal_create_eq RPC: " << status.error_message() << endl;
    }

    cout << "lib_driver.cc: hal_create_eq comp status: " << comp->status << endl;

    *item->done = 1;
    return;
}

void hal_create_mr (struct create_mr_cmd  *cmd,
                    struct create_mr_comp *comp,
                    hal_req_resp_t        *item)
{
    shared_ptr<Rdma::Stub> rdma_svc = GetRdmaStub();

    ClientContext context;
    RdmaMemRegRequestMsg request;
    RdmaMemRegResponseMsg response;
    
    RdmaMemRegSpec *spec = request.add_request();
    spec->set_hw_lif_id(cmd->lif+lif_base);
    spec->set_pd(cmd->pd_num);
    spec->set_va(cmd->start);
    spec->set_len(cmd->length);
    spec->set_ac_local_wr(cmd->access_flags & AC_LOCAL_WRITE);
    spec->set_ac_remote_wr(cmd->access_flags & AC_REMOTE_WRITE);
    spec->set_ac_remote_rd(cmd->access_flags & AC_REMOTE_READ);
    spec->set_ac_remote_atomic(cmd->access_flags & AC_REMOTE_ATOMIC);
    spec->set_lkey(cmd->lkey);
    spec->set_rkey(cmd->rkey);
    spec->set_hostmem_pg_size(cmd->page_size);

    cout << "lib_driver.cc: " << __FUNCTION__ << " VA %x len %x" << cmd->start << cmd->length << endl;
    cout << "lib_driver.cc: " << __FUNCTION__ << " PA Table:" << endl;
    
    //Set the va to pa translations.
    for (int i = 0; i < (int)cmd->nchunks; i++) {
        spec->add_va_pages_phy_addr(item->pt_table[i]);
        cout << item->pt_table[i] << endl;
    }

    Status status = rdma_svc->RdmaMemReg(&context, request, &response);
    if (status.ok()) {
        //This loop gets executed only once.
        for (int i = 0; i < response.response().size(); i++) {
            RdmaMemRegResponse mr_response = response.response(i);
            comp->status = mr_response.api_status();
            cout << "lib_driver.cc: hal_create_mr SUCCESS\n" << endl;
        }
    } else {
        cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
        comp->status = status.error_code();
        // Set done bit before returning
        *item->done = 1;
        return;
    }

    // Set done bit before returning
    *item->done = 1;

    return;
}

void hal_create_cq (struct create_cq_cmd  *cmd,
                    struct create_cq_comp *comp,
                    hal_req_resp_t        *item)                    
{
    shared_ptr<Rdma::Stub> rdma_svc = GetRdmaStub();

    
    ClientContext context1;

    RdmaMemRegRequestMsg request;
    RdmaMemRegResponseMsg response;

    /*
     * Ideally we are supposed to have a single HAL command for CQ and QP
     * implementation, but its not the case right now. We need to change it
     * later. For now call MR registration first and then create_cq 
     */
    RdmaMemRegSpec *spec = request.add_request();
    spec->set_hw_lif_id(cmd->lif_id+lif_base);
    //CQ does not have a PD associated. For now anyway we support only
    //one PD=1
    spec->set_pd(1);
    spec->set_va(cmd->cq_va);
    spec->set_len(cmd->va_len);
    spec->set_ac_local_wr(1);
    spec->set_ac_remote_wr(0);
    spec->set_ac_remote_rd(0);
    spec->set_ac_remote_atomic(0);
    spec->set_lkey(cmd->cq_lkey);
    spec->set_hostmem_pg_size(cmd->host_pg_size);

    //Set the va to pa translations.
    for (int i = 0; i < (int)cmd->pt_size; i++) {
        spec->add_va_pages_phy_addr(item->pt_table[i]);
    }

    Status status = rdma_svc->RdmaMemReg(&context1, request, &response);
    if (!status.ok()) {
        cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
        comp->status = status.error_code();
        // Set done bit before returning
        *item->done = 1;
        return;
    }

    cout << "lib_driver.cc: hal_create_mr SUCCESS\n" << endl;
 
    /*
     * create CQ
     */
    ClientContext context2;    
    RdmaCqRequestMsg cq_request;
    RdmaCqResponseMsg cq_response;

    RdmaCqSpec *cq_spec = cq_request.add_request();

    cq_spec->set_cq_num(cmd->cq_num);
    cq_spec->set_hw_lif_id(cmd->lif_id+lif_base);
    cq_spec->set_cq_wqe_size(cmd->cq_wqe_size);
    cq_spec->set_num_cq_wqes(cmd->num_cq_wqes);
    cq_spec->set_hostmem_pg_size(cmd->host_pg_size);
    cq_spec->set_cq_lkey(cmd->cq_lkey);

    status = rdma_svc->RdmaCqCreate(&context2, cq_request, &cq_response);
    if (status.ok()) {
        RdmaCqResponse cq_resp = cq_response.response(0);
        comp->status = cq_resp.api_status();
        cout << "lib_driver.cc: hal_create_cq SUCCESS\n" << endl;
    } else {
        cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
        comp->status = status.error_code();
    }

    comp->qtype = Q_TYPE_RDMA_CQ;

    // Set done bit before returning
    *item->done = 1;
    
    return;
}

void hal_create_qp (struct create_qp_cmd *cmd,
                    struct create_qp_comp *comp,
                    hal_req_resp_t        *item)
{
    shared_ptr<Rdma::Stub> rdma_svc = GetRdmaStub();

    /*
     * create CQ
     */
    ClientContext context;
    RdmaQpRequestMsg request;
    RdmaQpResponseMsg response;

    RdmaQpSpec *spec = request.add_request();

    spec->set_qp_num(cmd->qp_num);
    spec->set_hw_lif_id(cmd->lif_id+lif_base);
    spec->set_sq_wqe_size(cmd->sq_wqe_size);
    spec->set_rq_wqe_size(cmd->rq_wqe_size);    
    spec->set_num_sq_wqes(cmd->num_sq_wqes);
    spec->set_num_rq_wqes(cmd->num_rq_wqes);
    spec->set_num_rsq_wqes(cmd->num_rsq_wqes);
    spec->set_num_rrq_wqes(cmd->num_rrq_wqes);
    spec->set_pd(cmd->pd);
    spec->set_pmtu(cmd->pmtu);
    spec->set_hostmem_pg_size(cmd->host_pg_size);
    spec->set_svc((RdmaServiceType)cmd->service);
    spec->set_atomic_enabled(0);
    spec->set_sq_lkey(cmd->sq_lkey);
    spec->set_rq_lkey(cmd->rq_lkey);
    spec->set_sq_cq_num(cmd->sq_cq_num);
    spec->set_rq_cq_num(cmd->rq_cq_num);

    Status status = rdma_svc->RdmaQpCreate(&context, request, &response);
    if (status.ok()) {
        RdmaQpResponse resp = response.response(0);
        comp->status = resp.api_status();
        comp->sq_qtype = Q_TYPE_RDMA_SQ;
        comp->rq_qtype = Q_TYPE_RDMA_RQ;
        cout << "lib_driver.cc: hal_create_qp SUCCESS\n" << endl;
    } else {
        cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
        comp->status = status.error_code() ;
        // Set done bit before returning
        *item->done = 1;
        return;
    }

    // Set done bit before returning
    *item->done = 1;

    return;
}


#define 	IB_QP_AV		   (1<<7)
#define     IB_QP_DEST_QPN     (1<<20)
#define     IB_QP_RQ_PSN	   (1<<12)
#define     IB_QP_SQ_PSN	   (1<<16)
void hal_modify_qp (struct modify_qp_cmd *cmd,
                    struct modify_qp_comp *comp,
                    hal_req_resp_t        *item)
{
    shared_ptr<Rdma::Stub> rdma_svc = GetRdmaStub();

    /*
     * Update QP
     */

    if (cmd->attr_mask & IB_QP_AV) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(cmd->qp_num);
        spec->set_hw_lif_id(cmd->lif_id+lif_base);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_HEADER_TEMPLATE);
        
        spec->set_header_template(item->header, item->header_template_size);

        cout << "lib_driver.cc: hal_modify_qp AV:" << endl;
        for (int i = 0; i < item->header_template_size; i++) {
            printf("%x ", item->header[i]);
        }
        cout << endl;
        Status status = rdma_svc->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse resp = response.response(0);
            comp->status = resp.api_status();
            cout << "lib_driver.cc: hal_modify_qp AV SUCCESS\n" << endl;
        } else {
            cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
            comp->status = status.error_code();

            // Set done bit before returning
            *item->done = 1;
            
            return;
        }
    }

    if (cmd->attr_mask & IB_QP_DEST_QPN) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(cmd->qp_num);
        spec->set_hw_lif_id(cmd->lif_id+lif_base);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_DEST_QP);
        spec->set_dst_qp_num(cmd->dest_qp_num);
        Status status = rdma_svc->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse resp = response.response(0);
            comp->status = resp.api_status();
            cout << "lib_driver.cc: hal_modify_qp Dest QPN SUCCESS\n" << endl;
        } else {
            cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
            comp->status = status.error_code();

            // Set done bit before returning
            *item->done = 1;
            
            return;
        }
    }

    if (cmd->attr_mask & IB_QP_RQ_PSN) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(cmd->qp_num);
        spec->set_hw_lif_id(cmd->lif_id+lif_base);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_E_PSN);
        spec->set_e_psn(cmd->e_psn);
        Status status = rdma_svc->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse resp = response.response(0);
            comp->status = resp.api_status();
            cout << "lib_driver.cc: hal_modify_qp E_PSN SUCCESS\n" << endl;
        } else {
            cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
            comp->status = status.error_code();

            // Set done bit before returning
            *item->done = 1;
            
            return;
        }
    }

    if (cmd->attr_mask & IB_QP_SQ_PSN) {
        ClientContext context;
        RdmaQpUpdateRequestMsg request;
        RdmaQpUpdateResponseMsg response;

        RdmaQpUpdateSpec *spec = request.add_request();

        spec->set_qp_num(cmd->qp_num);
        spec->set_hw_lif_id(cmd->lif_id+lif_base);
        spec->set_oper(RDMA_UPDATE_QP_OPER_SET_TX_PSN);
        spec->set_tx_psn(cmd->sq_psn);
        Status status = rdma_svc->RdmaQpUpdate(&context, request, &response);
        if (status.ok()) {
            RdmaQpUpdateResponse resp = response.response(0);
            comp->status = resp.api_status();
            cout << "lib_driver.cc: hal_modify_qp TX_PSN SUCCESS\n" << endl;
        } else {
            cout << "lib_driver.cc: " << status.error_code() << ": " << status.error_message() << endl;
            comp->status = status.error_code();

            // Set done bit before returning
            *item->done = 1;
            
            return;
        }
    }
    
    // Set done bit before returning
    *item->done = 1;

    return;
}

class HalReqThread
{
public:
    HalReqThread(HalReqBuffer* buffer)
        {
            this->buffer_ = buffer;
        }
    void run() {
        while (true) {
            hal_req_resp_t req = buffer_->remove();

            std::cout << "Processing Req opcode %d: " << req.cmd.opcode << std::endl;

            switch(req.cmd.opcode)
            {
            case CMD_OPCODE_RDMA_CREATE_EQ:
                hal_create_eq((struct create_eq_cmd *)&req.cmd,
                              (struct admin_comp *)&req.comp,
                              &req);
                break;

            case CMD_OPCODE_RDMA_CREATE_MR:
                hal_create_mr((struct create_mr_cmd *)&req.cmd,
                              (struct create_mr_comp *)&req.comp,
                              &req);
                break;

            case CMD_OPCODE_RDMA_CREATE_CQ:
                hal_create_cq((struct create_cq_cmd *)&req.cmd,
                              (struct create_cq_comp *)&req.comp,
                              &req);
                break;

            case CMD_OPCODE_RDMA_CREATE_QP:
                hal_create_qp((struct create_qp_cmd *)&req.cmd,
                              (struct create_qp_comp *)&req.comp,
                              &req);
                break;

            case CMD_OPCODE_RDMA_MODIFY_QP:
                hal_modify_qp((struct modify_qp_cmd *)&req.cmd,
                              (struct modify_qp_comp *)&req.comp,
                              &req);
                break;

            default:
                std::cout << "HalReqThread: default case... missing something?" << std::endl;
                break;
            }
        }
    }
private:
    HalReqBuffer *buffer_;
};


HalReqBuffer reqBuf;
HalReqThread reqThr(&reqBuf);
std::thread *consumer_thread;

extern "C" void hal_set_lif_base (uint32_t lif)
{
    lif_base = lif;
}

extern "C" void init_lib_driver (void)
{
    /*
     * Lets create the thread that interacts with HAL in asynchronous fashion
     */
    consumer_thread = new std::thread(&HalReqThread::run, &reqThr);
}

extern "C" void hal_create_eq_wrapper (struct create_eq_cmd  *cmd,
                                       struct admin_comp     *comp,
                                       u_int32_t             *done)
{
    hal_req_resp_t item;

    memset(&item, 0, sizeof(item));
    std::cout << "Queing Req with opcode %d: " << cmd->opcode << std::endl;
    memcpy(&item.cmd, cmd, sizeof(*cmd));
    memcpy(&item.comp, comp, sizeof(*comp));

    item.done = done;

    reqBuf.add(item);
    comp->status = 0;
}

extern "C" void hal_create_mr_wrapper (struct create_mr_cmd  *cmd,
                                       struct create_mr_comp *comp,
                                       u_int32_t             *done)
{
    hal_req_resp_t item;

    memset(&item, 0, sizeof(item));
    std::cout << "Queing Req with opcode %d: " << cmd->opcode << std::endl;
    memcpy(&item.cmd, cmd, sizeof(*cmd));
    memcpy(&item.comp, comp, sizeof(*comp));

    item.done = done;
    item.pt_table = new uint64_t [cmd->nchunks];

    simdev_read_host_mem(cmd->pt_dma, item.pt_table,
                         cmd->nchunks*sizeof(uint64_t));
    
    reqBuf.add(item);
    comp->status = 0;
}

extern "C" void hal_create_cq_wrapper (struct create_cq_cmd  *cmd,
                                       struct create_cq_comp *comp,
                                       u_int32_t             *done)
{
    hal_req_resp_t item;

    memset(&item, 0, sizeof(item));
    std::cout << "Queing Req with opcode %d: " << cmd->opcode << std::endl;
    memcpy(&item.cmd, cmd, sizeof(*cmd));
    memcpy(&item.comp, comp, sizeof(*comp));
    item.done = done;

    item.pt_table = new uint64_t [cmd->pt_size];

    simdev_read_host_mem(cmd->pt_base_addr, item.pt_table,
                         cmd->pt_size*sizeof(uint64_t));
    
    reqBuf.add(item);
    comp->status = 0;
    comp->qtype = Q_TYPE_RDMA_CQ;
}

extern "C" void hal_create_qp_wrapper (struct create_qp_cmd  *cmd,
                                       struct create_qp_comp *comp,
                                       u_int32_t             *done)
{
    hal_req_resp_t item;

    memset(&item, 0, sizeof(item));
    std::cout << "Queing Req with opcode %d: " << cmd->opcode << std::endl;
    memcpy(&item.cmd, cmd, sizeof(*cmd));
    memcpy(&item.comp, comp, sizeof(*comp));
    item.done = done;
    
    reqBuf.add(item);
    comp->status = 0;
    comp->sq_qtype = Q_TYPE_RDMA_SQ;
    comp->rq_qtype = Q_TYPE_RDMA_RQ;
}

extern "C" void hal_modify_qp_wrapper (struct modify_qp_cmd  *cmd,
                                       struct modify_qp_comp *comp,
                                       u_int32_t             *done)
{
    hal_req_resp_t item;

    std::cout << "Queing Req with opcode %d: " << cmd->opcode << std::endl;
    
    memset(&item, 0, sizeof(item));
    memcpy(&item.cmd, cmd, sizeof(*cmd));
    memcpy(&item.comp, comp, sizeof(*comp));
    item.done = done;

    if (cmd->attr_mask & IB_QP_AV) {
        item.header = new unsigned char[(cmd->header_template_size/sizeof(uint64_t)+1)*sizeof(uint64_t)];
        simdev_read_host_mem(cmd->header_template, item.header, (cmd->header_template_size/sizeof(uint64_t)+1)*sizeof(uint64_t));
        item.header_template_size = cmd->header_template_size;
    }
    reqBuf.add(item);
    comp->status = 0;
}
