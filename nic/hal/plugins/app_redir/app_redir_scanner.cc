
#include "nic/include/hal.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "nic/third-party/snort3/export/include/snort_api.h"
#include "app_redir_scanner.hpp"

#define PEN_SNORT_CFG_PATH "/sw/nic/third-party/snort3/export/lua/snort.lua"  // TODO
//#define PEN_SNORT_CFG_PATH "./snort.lua"  // TODO
#define PEN_SNORT_THREAD_COUNT (1 + hal::HAL_THREAD_ID_FTE_MAX - hal::HAL_THREAD_ID_FTE_MIN)

// defined in milliseconds, keep it low since datapath needs a tight loop
#define SNORT_PERIODIC_INTERVAL  1

// arbitrary unique number for easy debugging
#define SNORT_PERIODIC_TIMER_ID  0x6789

static void* scanner_periodic_timer = nullptr;

volatile static bool scanner_thread_initialized[hal::HAL_THREAD_ID_MAX] = { false };

void scanner_periodic_cb(uint32_t timer_id, void* ctx)
{
    // invoked by timer wheel in FTE control thread
    assert(timer_id == SNORT_PERIODIC_TIMER_ID);
    scanner_run();
}

hal_ret_t scanner_init(int thread_num) {
    if (thread_num >= hal::HAL_THREAD_ID_MAX) {
        HAL_TRACE_DEBUG("illegal scanner thread id {}", thread_num);
        return HAL_RET_ERR;
    }

    if (scanner_thread_initialized[thread_num]) {
        HAL_TRACE_DEBUG("scanner thread {} already initialized", thread_num);
        return HAL_RET_OK;
    }
    if (thread_num == hal::HAL_THREAD_ID_CFG) {
        // TODO: any non-thread init can go here
    } else if (thread_num == hal::HAL_THREAD_ID_PERIODIC) {
        std::string lua_path(std::getenv("SNORT_LUA_PATH"));
        if (lua_path.length() == 0) {
            HAL_TRACE_DEBUG("missing environment variable SNORT_LUA_PATH");
            goto error;
        }
        lua_path += "snort.lua";

        if (init_main_thread(PEN_SNORT_THREAD_COUNT, lua_path.c_str()) != 0) {
            goto error;
        }
        if (hal::periodic::periodic_thread_is_running()) {
            scanner_periodic_timer = hal::periodic::periodic_timer_schedule(
                    SNORT_PERIODIC_TIMER_ID, SNORT_PERIODIC_INTERVAL, nullptr,
                    scanner_periodic_cb, true);
        } // else this is probably running in gtest
    } else if (thread_num >= hal::HAL_THREAD_ID_FTE_MIN &&
               thread_num <= hal::HAL_THREAD_ID_FTE_MAX) {
        if (!scanner_thread_initialized[hal::HAL_THREAD_ID_PERIODIC]) {
            HAL_TRACE_DEBUG("cannot initialize scanner pkt thread before scanner periodic thread");
            assert(0);
            goto error;
        }
        if (init_pkt_thread(thread_num - hal::HAL_THREAD_ID_FTE_MIN) != 0) {
            goto error;
        }
    } else {
        HAL_TRACE_DEBUG("illegal scanner thread id {}", thread_num);
        goto error;
    }

    HAL_TRACE_DEBUG("scanner thread {} successfully initialized", thread_num);
    scanner_thread_initialized[thread_num] = true;

    return HAL_RET_OK;

error:
    HAL_TRACE_DEBUG("scanner thread {} initialization failed", thread_num);
    return HAL_RET_ERR;
}

static int scanner_pkt_thread_init_count() {
    int ret = 0;
    for (int tid = hal::HAL_THREAD_ID_FTE_MIN; tid <= hal::HAL_THREAD_ID_FTE_MAX; tid++) {
        if (scanner_thread_initialized[tid]) {
	    ret++;
        }
    }
    return ret;
}

hal_ret_t scanner_cleanup(int thread_num) {
    if (thread_num >= hal::HAL_THREAD_ID_MAX) {
        HAL_TRACE_DEBUG("illegal scanner thread id {}", thread_num);
        return HAL_RET_ERR;
    }

    if (!scanner_thread_initialized[thread_num]) {
        HAL_TRACE_DEBUG("scanner thread {} already destroyed, or never initialized", thread_num);
        return HAL_RET_OK;
    }

    if (thread_num == hal::HAL_THREAD_ID_CFG) {
        // TODO: any non-thread cleanup here
    } else if (thread_num == hal::HAL_THREAD_ID_PERIODIC) {
        if (scanner_pkt_thread_init_count() > 0) {
            HAL_TRACE_DEBUG("cannot destroy periodic scanner thread before pkt thread cleanup");
            assert(0);
            goto error;
        }
        if (scanner_periodic_timer) {
            hal::periodic::periodic_timer_delete(scanner_periodic_timer);
            scanner_periodic_timer = nullptr;
        }
        if (cleanup_main_thread() != 0) {
            goto error;
        }
    } else if (thread_num >= hal::HAL_THREAD_ID_FTE_MIN &&
               thread_num <= hal::HAL_THREAD_ID_FTE_MAX) {
        if (!scanner_thread_initialized[hal::HAL_THREAD_ID_PERIODIC]) {
            HAL_TRACE_DEBUG("cannot destroy scanner pkt thread while periodic thread not running");
            assert(0);
            goto error;
        }
        if (cleanup_pkt_thread(thread_num - hal::HAL_THREAD_ID_FTE_MIN) != 0) {
            goto error;
        }
    } else {
        HAL_TRACE_DEBUG("illegal scanner thread id {}", thread_num);
        goto error;
    }

    HAL_TRACE_DEBUG("scanner thread {} successfully destroyed", thread_num);
    scanner_thread_initialized[thread_num] = false;

    return HAL_RET_OK;

error:
    return HAL_RET_ERR;
}

static hal::appid_id_t scanner_appid_to_local_id(int appid)
{
    // TODO: Snort to Pensando appid mapping
    return (hal::appid_id_t) appid;
}

#define MAX_SCANNER_APPID_STATE 4
static hal::appid_state_t appid_state_map[MAX_SCANNER_APPID_STATE] = {
    hal::APPID_STATE_NOT_NEEDED, // APPID_DISCO_STATE_NONE
    hal::APPID_STATE_IN_PROGRESS, // APPID_DISCO_STATE_DIRECT, used for client discovery only
    hal::APPID_STATE_IN_PROGRESS, // APPID_DISCO_STATE_STATEFUL
    hal::APPID_STATE_FOUND, // APPID_DISCO_STATE_FINISHED
};

static hal::appid_state_t scanner_appid_state_to_local_state(int state)
{
    if (state < MAX_SCANNER_APPID_STATE) {
        return appid_state_map[state];
    }
    return hal::APPID_STATE_ABORT;
}

static void scanner_flow_info_to_appid_info(SnortFlowInfo& flow_info, fte::appid_info_t& appid_info)
{
    appid_info_init(appid_info);
    for (int i = 0; i < NUM_APP_IDS; i++) {
        if (flow_info.appids[i]) {
            appid_info_set_id(appid_info, scanner_appid_to_local_id(flow_info.appids[i]));
        }
    }
    appid_info.state_ = scanner_appid_state_to_local_state(flow_info.app_detection_status);
    appid_info.cleanup_handle_ = flow_info.flow_handle;
}

/* background control thread runs this version of scanner_run */
hal_ret_t scanner_run() {
    assert(scanner_thread_initialized[hal::HAL_THREAD_ID_PERIODIC]);
    if (run_main_thread_loop() == 0) {
        return HAL_RET_OK;
    }
    return HAL_RET_ERR;
}

/* submit a packet to scanner and run it */
hal_ret_t scanner_run(fte::appid_info_t& appid_info, uint8_t* pkt, uint32_t pkt_len, void* ctx) {
    void *snort_flow_handle = nullptr;
    hal::utils::thread* t = hal::utils::thread::current_thread();
    uint32_t tid = t ? t->thread_id() : hal::HAL_THREAD_ID_FTE_MIN;
    assert(tid >= hal::HAL_THREAD_ID_FTE_MIN && tid <= hal::HAL_THREAD_ID_FTE_MAX);
    assert(scanner_thread_initialized[tid]);

    tid -= hal::HAL_THREAD_ID_FTE_MIN;

    // Ignore return value, which will always be -1
    run_pkt_thread_loop(tid, pkt, pkt_len, &snort_flow_handle, ctx);

    if (snort_flow_handle) {
        SnortFlowInfo flow_info;
        appid_info.cleanup_handle_ = snort_flow_handle;
	if (0 == get_flow_info_by_handle(snort_flow_handle, &flow_info)) {
            scanner_flow_info_to_appid_info(flow_info, appid_info);
        }
    }

    return HAL_RET_OK;
}

hal_ret_t scanner_get_appid_info(const hal::flow_key_t& key, fte::appid_info_t& appid_info) {
    struct SnortFlowInfo flow_info;

    if (key.flow_type == hal::FLOW_TYPE_L2) {
        // appid is only for IP traffic right now */
        return HAL_RET_ERR;
    }

    if (0 == get_flow_info_by_key(key.proto, key.flow_type == hal::FLOW_TYPE_V6,
                 key.sip.v6_addr.addr8, key.dip.v6_addr.addr8, key.sport, key.dport,
                 0 /* TODO: vlan_id */, (uint16_t) key.vrf_id, &flow_info)) {
        scanner_flow_info_to_appid_info(flow_info, appid_info);
        return HAL_RET_OK;
    }

    return HAL_RET_ERR;
}

hal_ret_t scanner_cleanup_flow(void* flow_handle) {
    if (cleanup_flow(flow_handle) != 0) {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}
