#ifndef FTE_EP_LEARN_DHCP_DHCP_TRANS_H_
#define FTE_EP_LEARN_DHCP_DHCP_TRANS_H_
#include <netinet/in.h>
#include <string>
#include <netinet/if_ether.h>
#include "nic/utils/fsm/fsm.hpp"
#include "nic/fte/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/ht.hpp"
#include "nic/include/twheel.hpp"
#include "../common/trans.hpp"

#define HAL_MAX_DHCP_TRANS 512
#define DHCP_TIMER_ID 255

using hal::utils::fsm_state_t;
using hal::utils::fsm_state_ctx;
using hal::utils::fsm_event_data;
using hal::utils::fsm_transition_t;
using hal::utils::fsm_transition_func;
using hal::utils::fsm_state_func;
using hal::utils::fsm_state_machine_t;
using hal::utils::fsm_state_machine_def_t;
using hal::utils::fsm_timer_t;
using hal::utils::fsm_state_timer_ctx;
using hal::utils::twheel;
struct packet; //forward declaration.

namespace hal {
namespace periodic {
extern twheel* g_twheel;
}  // namespace periodic
}  // namespace hal

namespace hal {
namespace eplearn {

class dhcp_trans_t;  // forward

// DHCP transaction key
typedef struct dhcp_trans_key_s {
    vrf_id_t vrf_id;    //tenant id
    uint32_t xid;         // transaction ID.
    mac_addr_t mac_addr;  // MAC address of the endpoint

} __PACK__ dhcp_trans_key_t;

enum dhcp_fsm_state_t {
    DHCP_INIT,
    DHCP_SELECTING,
    DHCP_REQUESTING,
    DHCP_BOUND,
    DHCP_RENEWING,
    DHCP_DONE,
};

#define DHCP_FSM_EVENT_ENTRIES(ENTRY)                      \
    ENTRY(DHCP_DISCOVER,       0, "DHCP_DISCOVER")         \
    ENTRY(DHCP_OFFER,          1, "DHCP_OFFER")            \
    ENTRY(DHCP_REQUEST,        2, "DHCP_REQUEST")          \
    ENTRY(DHCP_INFORM,         3, "DHCP_INFORM")           \
    ENTRY(DHCP_ACK,            4, "DHCP_ACK")              \
    ENTRY(DHCP_NACK,           5, "DHCP_NACK")             \
    ENTRY(DHCP_DECLINE,        6, "DHCP_DECLINE")          \
    ENTRY(DHCP_RELEASE,        7, "DHCP_RELEASE")          \
    ENTRY(DHCP_INVALID_PACKET, 8, "DHCP_INVALID_PACKET")   \
    ENTRY(DHCP_ERROR,          9, "DHCP_ERROR")            \
    ENTRY(DHCP_TIMEOUT,        10, "DHCP_TIMEOUT")         \

DEFINE_ENUM(dhcp_fsm_event_t, DHCP_FSM_EVENT_ENTRIES)

class dhcp_ctx {
   public:
    uint32_t xid_;
    uint32_t server_identifer_;
    uint32_t renewal_time_;
    uint32_t rebinding_time_;
    uint32_t lease_time_;
    struct in_addr yiaddr_;
    struct in_addr subnet_mask_;
    unsigned char chaddr_[16];
    void init(struct packet* dhcp_packet);
};

struct dhcp_event_data {
    const struct packet *decoded_packet;
    fte::ctx_t    *fte_ctx;
};

class dhcp_trans_t : public trans_t  {
public:public:
    static ht *dhcplearn_key_ht(void) { return dhcplearn_key_ht_; }
    static ht *dhcplearn_ip_entry_ht(void) { return dhcplearn_ip_entry_ht_; }
private:
    dhcp_trans_key_t trans_key_;
    class dhcp_fsm_t;  // forward declaration.
    class dhcp_timer_t;  // Forward declaration.
    static dhcp_fsm_t* dhcp_fsm_;
    static trans_timer_t* dhcp_timer_;
    static slab *dhcplearn_slab_;
    static ht *dhcplearn_key_ht_;
    static ht *dhcplearn_ip_entry_ht_;

    dhcp_ctx ctx_;

    ht_ctxt_t ht_ctxt_;           // id based hash table ctxt
    ht_ctxt_t ip_entry_ht_ctxt_;  // IP based hash table ctxt

    class dhcp_fsm_t {
        void _init_state_machine();

        void set_state_machine(fsm_state_machine_def_t sm_def) {
            this->sm_def = sm_def;
        }

        bool process_dhcp_discover(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dhcp_inform(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dhcp_request(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dhcp_release(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dhcp_request_after_bound(fsm_state_ctx ctx,
                                              fsm_event_data data);
        bool process_dhcp_bound_timeout(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dhcp_ack(fsm_state_ctx ctx, fsm_event_data data);
        bool process_dhcp_offer(fsm_state_ctx ctx, fsm_event_data data);

        void bound_entry_func(fsm_state_ctx ctx);

       public:
        fsm_state_machine_def_t sm_def;
        dhcp_fsm_t() { this->_init_state_machine(); }
    };
    static fsm_state_machine_def_t* get_sm_def_func() {
        return &dhcp_fsm_->sm_def;
    }
    static fsm_timer_t* get_timer_func() { return dhcp_timer_; }
    static dhcp_trans_t* dhcp_trans_alloc(void) {
        dhcp_trans_t* dhcp_trans;

        dhcp_trans = (dhcp_trans_t*)dhcplearn_slab_->alloc();
        if (dhcp_trans == NULL) {
            return NULL;
        }
        return dhcp_trans;
    }

    static dhcp_trans_t* dhcp_trans_init(dhcp_trans_t* dhcp_trans) {
        if (!dhcp_trans) {
            return NULL;
        }

        // initialize meta information
        dhcp_trans->ht_ctxt_.reset();
        dhcp_trans->ip_entry_ht_ctxt_.reset();

        return dhcp_trans;
    }

    static dhcp_trans_t* dhcp_trans_alloc_init(void) {
        return dhcp_trans_init(dhcp_trans_alloc());
    }

   public:
    void* operator new(size_t size) {
        void* p = dhcp_trans_alloc_init();
        return p;
    }
    void operator delete(void* p) { dhcp_trans_free((dhcp_trans_t*)p); }

    static inline hal_ret_t dhcp_trans_free(dhcp_trans_t* dhcp_trans) {
        dhcplearn_slab_->free(dhcp_trans);
        return HAL_RET_OK;
    }
    dhcp_trans_key_t* trans_key_ptr() { return &trans_key_; }
    static void init_dhcp_trans_key(const uint8_t* chaddr, uint32_t xid,
                                    const ep_t *ep,
                                    dhcp_trans_key_t* trans_key_);
    //static void process_transaction(dhcp_trans_t* trans, dhcp_fsm_event_t event,
      //                              fsm_event_data data);
    static dhcp_trans_t* find_dhcptrans_by_id(dhcp_trans_key_t id) {
        return (dhcp_trans_t*)dhcplearn_key_ht_->lookup(&id);
    }

    static dhcp_trans_t* find_dhcptrans_by_handle(hal_handle_t handle) {
        return (dhcp_trans_t*)dhcplearn_ip_entry_ht_->lookup(
            &handle);
    }

    explicit dhcp_trans_t(struct packet* dhcp_packet, fte::ctx_t& ctx);
    void process_event(dhcp_fsm_event_t event, fsm_event_data data);
    const dhcp_ctx* get_ctx() { return &this->ctx_; }
    dhcp_fsm_state_t get_state() { return (dhcp_fsm_state_t)this->sm_->get_state(); }
    virtual void log_info(const char *message) {
        HAL_TRACE_INFO("(tenant id : {}, xid : {}, "
                "macaddr : {}, ip : {}) : {} ",
                this->trans_key_ptr()->vrf_id, this->trans_key_ptr()->xid,
                macaddr2str(this->trans_key_ptr()->mac_addr),
                        ipaddr2str(&ip_entry_key_ptr()->ip_addr), message);
    }
    virtual void log_error(const char *message) {
        HAL_TRACE_ERR("(tenant id : {}, xid : {}, "
                "macaddr : {}, ip : {}) : {} ",
                this->trans_key_ptr()->vrf_id, this->trans_key_ptr()->xid,
                macaddr2str(this->trans_key_ptr()->mac_addr),
                ipaddr2str(&ip_entry_key_ptr()->ip_addr), message);
    }

    virtual ~dhcp_trans_t();
} __PACK__;

void* dhcptrans_get_key_func(void* entry);
uint32_t dhcptrans_compute_hash_func(void* key, uint32_t ht_size);
bool dhcptrans_compare_key_func(void* key1, void* key2);

void* dhcptrans_get_handle_key_func(void* entry);
uint32_t dhcptrans_compute_handle_hash_func(void* key, uint32_t ht_size);
bool dhcptrans_compare_handle_key_func(void* key1, void* key2);

}  // namespace eplearn
}  // namespace hal

#endif /* FTE_EP_LEARN_DHCP_DHCP_TRANS_H_ */
