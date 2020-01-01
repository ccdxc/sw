#ifndef __ACCEL_LIF_UTILS_HPP__
#define __ACCEL_LIF_UTILS_HPP__

#include <utility>
#include <map>

#ifndef USEC_PER_SEC
#define USEC_PER_SEC    1000000L
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#endif

class AccelLif;

namespace AccelLifUtils {

/**
 * Generic timestamp and expiry interval
 */
typedef struct {
    uint64_t    timestamp;
    uint64_t    expiry;
} accel_timestamp_t;

static inline uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static inline void
time_expiry_set(accel_timestamp_t& ts,
                uint64_t expiry)
{
    ts.timestamp = timestamp();
    ts.expiry = expiry;
}

static inline bool
time_expiry_check(const accel_timestamp_t& ts)
{
    return (ts.expiry == 0) ||
           ((timestamp() - ts.timestamp) > ts.expiry);
}

/**
 * FW-Driver notify queue
 */
class NotifyQ {
public:
    NotifyQ(const std::string& name,
            PdClient *pd,
            uint64_t lif_id,
            uint32_t tx_qtype,
            uint32_t tx_qid,
            uint64_t intr_base,
            uint8_t  admin_cosA,
            uint8_t  admin_cosB,
            bool host_dev = true);
    ~NotifyQ();

    bool TxQInit(const notifyq_init_cmd_t *init_cmd,
                 uint32_t desc_size);
    bool TxQPost(const void *desc);
    bool TxQReset(void);

private:
    const std::string&  name;
    PdClient            *pd;

    uint64_t            lif_id;
    uint32_t            tx_qtype;
    uint32_t            tx_qid;
    uint32_t            intr_base;
    uint8_t             admin_cosA;
    uint8_t             admin_cosB;
    bool                host_dev;

    /*
     * NotifyQ in host follows qcq structure where the host ring space
     * consists of a command (Rx) ring and a completion (Tx) ring.
     * Only the completion (Tx) ring is used in the FW-to-host direction.
     */
    uint64_t            tx_ring_base;
    uint32_t            tx_ring_size;
    uint32_t            tx_alloc_size;
    uint32_t            tx_desc_size;
    uint32_t            tx_head;
};

/**
 * Simple Error Logging Control
 */
typedef enum {
    DETECTION_LOG_ST_INITIAL,
    DETECTION_LOG_ST_ENTER,
    DETECTION_LOG_ST_REQUEST,
    DETECTION_LOG_ST_SUPPRESS,
} detection_log_state_t;

class DetectionLogControl {
public:
    DetectionLogControl(uint64_t duration);

    void Enter(void);
    bool Request(void);
    void Leave(void);

private:
    uint64_t                    duration;
    detection_log_state_t       state;
    accel_timestamp_t           ts;
};

/**
 * Hardware Monitor
 */
class HwMonitor {
public:
    HwMonitor(AccelLif& lif,
              EV_P);
    ~HwMonitor();

    void ErrPollStart(void);
    void ErrPollStop(void);
    void reset_log_reason_eval(const accel_rgroup_rmisc_rsp_t& misc,
                               uint32_t *ret_reason_code);

    friend void mon_rmisc_rsp_cb(void *user_ctx,
                                 const accel_rgroup_rmisc_rsp_t& misc);
    static void ErrPoll(void *obj);

private:
    AccelLif&           lif;
    DetectionLogControl detection_log;
    uint32_t            reason_code;

    EV_P;
    evutil_timer        err_timer;
    bool                err_timer_started;
};

} // AccelLifUtils

#endif

