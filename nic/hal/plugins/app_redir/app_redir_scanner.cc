
#include "nic/include/hal.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "nic/third-party/snort3/export/include/snort_api.h"
#include "app_redir_scanner.hpp"
#include <dlfcn.h>

namespace hal {
namespace app_redir {

#define PEN_SNORT_THREAD_COUNT (1 + hal::HAL_THREAD_ID_FTE_MAX - hal::HAL_THREAD_ID_FTE_MIN)

// defined in milliseconds, keep it low since datapath needs a tight loop
// TODO: current twheel granularity is 250ms, so this value is very high for now
// TODO: probably need to use avoid using twheel entirely at some point
#define SNORT_PERIODIC_INTERVAL  250

// arbitrary unique number for easy debugging
#define SNORT_PERIODIC_TIMER_ID  0x6789

static void* scanner_periodic_timer = nullptr;

static void* snort_lib_so = nullptr;
volatile static bool scanner_thread_initialized[hal::HAL_THREAD_ID_MAX] = { false };
volatile static uint32_t scanner_periodic_cb_count = 0;

// list of functions dynamically loaded from libsnort.so
#define SCANNER_SNORT_DLFN_LIST \
    SCANNER_SNORT_DLFN(init_main_thread);         \
    SCANNER_SNORT_DLFN(run_main_thread_loop);     \
    SCANNER_SNORT_DLFN(cleanup_main_thread);      \
    SCANNER_SNORT_DLFN(init_pkt_thread);          \
    SCANNER_SNORT_DLFN(run_pkt_thread_loop);      \
    SCANNER_SNORT_DLFN(get_pkt_thread_flow_info); \
    SCANNER_SNORT_DLFN(get_flow_info_by_handle);  \
    SCANNER_SNORT_DLFN(get_flow_info_by_key);     \
    SCANNER_SNORT_DLFN(cleanup_pkt_thread);       \
    SCANNER_SNORT_DLFN(cleanup_flow);             \
    SCANNER_SNORT_DLFN(cleanup_flow_by_key);

// struct of dynamically loaded function pointers
static struct {
#define SCANNER_SNORT_DLFN(fn_name) decltype(fn_name)* lib_##fn_name
SCANNER_SNORT_DLFN_LIST
#undef SCANNER_SNORT_DLFN
} snort_dl;

static bool scanner_dlopen()
{
    // clear function pointers
    memset(&snort_dl, 0, sizeof(snort_dl));

    // get libsnort path
    const char* str = std::getenv("SNORT_LUA_PATH");
    if (str == nullptr) {
        HAL_TRACE_WARN("missing environment variable SNORT_LUA_PATH");
        return false;
    }
    std::string lib_path {str};
    lib_path += "../bin/libsnort.so";

    // open libsnort
    HAL_TRACE_INFO("scanner_dlopen loading so {}", lib_path);
    snort_lib_so = dlopen(lib_path.c_str(), RTLD_NOW|RTLD_DEEPBIND); // TODO
    if (!snort_lib_so) {
        HAL_TRACE_ERR("scanner_dlopen failed {}", dlerror());
        return false;
    }

    // load symbols
    const char *dlsym_error;
#define SCANNER_SNORT_DLFN(fn_name) \
    snort_dl.lib_##fn_name = (decltype(fn_name)*) dlsym(snort_lib_so, #fn_name); \
    dlsym_error = dlerror(); \
    if (dlsym_error) { \
        HAL_TRACE_ERR("scanner: cannot load snort symbol {}, error {}", #fn_name, dlsym_error); \
        goto exit; \
    }
    SCANNER_SNORT_DLFN_LIST
#undef SCANNER_SNORT_DLFN
    return true;

exit:
    return false;
}

static bool scanner_dlclose()
{
    // clear function pointers
    memset(&snort_dl, 0, sizeof(snort_dl));

    // close libsnort
    HAL_TRACE_INFO("scanner_dlclose closing libsnort.so");
    int rc = dlclose(snort_lib_so);
    snort_lib_so = nullptr;
    return (rc == 0);
}

void scanner_periodic_cb(uint32_t timer_id, void* ctx)
{
    hal::utils::thread* t = hal::utils::thread::current_thread();
    if (t) {
        assert(t->thread_id() == hal::HAL_THREAD_ID_PERIODIC);
    }

    // invoked by timer wheel in FTE control thread
    assert(timer_id == SNORT_PERIODIC_TIMER_ID);
    scanner_periodic_cb_count++;
    scanner_run();
    scanner_periodic_cb_count++;
}

hal_ret_t scanner_init(int thread_num) {
    hal::utils::thread* t = hal::utils::thread::current_thread();
    uint32_t tid = t ? t->thread_id() : hal::HAL_THREAD_ID_MAX;

    if (thread_num >= hal::HAL_THREAD_ID_MAX) {
        HAL_TRACE_ERR("illegal scanner thread id {}", thread_num);
        return HAL_RET_ERR;
    }

    if (scanner_thread_initialized[thread_num]) {
        HAL_TRACE_WARN("scanner thread {} already initialized", thread_num);
        return HAL_RET_OK;
    }
    if (thread_num == hal::HAL_THREAD_ID_CFG) {
        // any non-thread init can go here
        if (!scanner_dlopen()) {
            goto error;
        }
    } else if (thread_num == hal::HAL_THREAD_ID_PERIODIC) {
        if (tid != hal::HAL_THREAD_ID_PERIODIC) {
            HAL_TRACE_WARN("initializing scanner periodic thread on wrong thread id {}", tid);
        }

        const char* str = std::getenv("SNORT_LUA_PATH");
        if (str == nullptr) {
            HAL_TRACE_WARN("missing environment variable SNORT_LUA_PATH");
            goto error;
        }
        std::string lua_path {str};
        lua_path += "snort.lua";

        if (!scanner_thread_initialized[hal::HAL_THREAD_ID_CFG]) {
            HAL_TRACE_WARN("cannot initialize scanner periodic thread before scanner config thread");
            goto error;
        }

        if (snort_dl.lib_init_main_thread(PEN_SNORT_THREAD_COUNT, lua_path.c_str(), NULL) != 0) {
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
            HAL_TRACE_WARN("cannot initialize scanner pkt thread before scanner periodic thread");
            goto error;
        }

        if (tid != (uint32_t) thread_num) {
            HAL_TRACE_WARN("initializing scanner pkt thread {} on wrong thread id {}", thread_num, tid);
        }

        if (snort_dl.lib_init_pkt_thread(thread_num - hal::HAL_THREAD_ID_FTE_MIN) != 0) {
            goto error;
        }
    } else {
        HAL_TRACE_ERR("illegal scanner thread id {}", thread_num);
        goto error;
    }

    HAL_TRACE_DEBUG("scanner thread {} successfully initialized", thread_num);
    scanner_thread_initialized[thread_num] = true;

    return HAL_RET_OK;

error:
    HAL_TRACE_INFO("scanner thread {} initialization failed", thread_num);
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
        HAL_TRACE_ERR("illegal scanner thread id {}", thread_num);
        return HAL_RET_ERR;
    }

    if (!scanner_thread_initialized[thread_num]) {
        HAL_TRACE_WARN("scanner thread {} already destroyed, or never initialized", thread_num);
        return HAL_RET_OK;
    }

    if (thread_num == hal::HAL_THREAD_ID_CFG) {
        // any non-thread cleanup here
        if (scanner_thread_initialized[hal::HAL_THREAD_ID_PERIODIC]) {
            HAL_TRACE_WARN("cannot destroy cfg scanner thread before periodic thread cleanup");
            goto error;
        }

        scanner_dlclose();
    } else if (thread_num == hal::HAL_THREAD_ID_PERIODIC) {
        if (scanner_pkt_thread_init_count() > 0) {
            HAL_TRACE_WARN("cannot destroy periodic scanner thread before pkt thread cleanup");
            goto error;
        }
        if (scanner_periodic_timer) {
            hal::periodic::periodic_timer_delete(scanner_periodic_timer);
            scanner_periodic_timer = nullptr;
        }
        if (snort_dl.lib_cleanup_main_thread() != 0) {
            goto error;
        }
    } else if (thread_num >= hal::HAL_THREAD_ID_FTE_MIN &&
               thread_num <= hal::HAL_THREAD_ID_FTE_MAX) {
        if (!scanner_thread_initialized[hal::HAL_THREAD_ID_PERIODIC]) {
            HAL_TRACE_WARN("cannot destroy scanner pkt thread while periodic thread not running");
            goto error;
        }
        if (snort_dl.lib_cleanup_pkt_thread(thread_num - hal::HAL_THREAD_ID_FTE_MIN) != 0) {
            goto error;
        }
    } else {
        HAL_TRACE_ERR("illegal scanner thread id {}", thread_num);
        goto error;
    }

    HAL_TRACE_DEBUG("scanner thread {} successfully destroyed", thread_num);
    scanner_thread_initialized[thread_num] = false;

    return HAL_RET_OK;

error:
    return HAL_RET_ERR;
}

static appid_id_t scanner_appid_to_local_id(int appid)
{
    // TODO: Snort to Pensando appid mapping
    return (appid_id_t) appid;
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
    if (!scanner_thread_initialized[hal::HAL_THREAD_ID_PERIODIC]) {
        HAL_TRACE_ERR("scanner_run: periodic scanner thread not initialized");
        return HAL_RET_ERR;
    }

    if (snort_dl.lib_run_main_thread_loop() == 0) {
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

    if (!scanner_thread_initialized[tid]) {
        HAL_TRACE_ERR("scanner_run: pkt scanner thread {} not initialized", tid);
        return HAL_RET_ERR;
    }

    tid -= hal::HAL_THREAD_ID_FTE_MIN;

    if (!hal::periodic::periodic_thread_is_running()) {
        HAL_TRACE_WARN("scanning pkt for appid, len {}, periodic thread IS NOT running, periodic_cb_count {}",
                       pkt_len, scanner_periodic_cb_count);
        // TODO: allow for now because of gtests, but eventually treat this as error
    } else {
        HAL_TRACE_DEBUG("scanning pkt for appid, len {}, periodic thread IS running, periodic_cb_count {}",
                        pkt_len, scanner_periodic_cb_count);
    }

    // Ignore return value, which will always be -1
    snort_dl.lib_run_pkt_thread_loop(tid, pkt, pkt_len, &snort_flow_handle, ctx);

    if (snort_flow_handle) {
        SnortFlowInfo flow_info;
        appid_info.cleanup_handle_ = snort_flow_handle;
	if (0 == snort_dl.lib_get_flow_info_by_handle(snort_flow_handle, &flow_info)) {
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

    if (0 == snort_dl.lib_get_flow_info_by_key(key.proto, key.flow_type == hal::FLOW_TYPE_V6,
                 key.sip.v6_addr.addr8, key.dip.v6_addr.addr8, key.sport, key.dport,
                 0 /* TODO: vlan_id */, (uint16_t) key.vrf_id, &flow_info)) {
        scanner_flow_info_to_appid_info(flow_info, appid_info);
        return HAL_RET_OK;
    }

    return HAL_RET_ERR;
}

hal_ret_t scanner_cleanup_flow(void* flow_handle) {
    if (snort_dl.lib_cleanup_flow(flow_handle) != 0) {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

} // namespace app_redir
} // namespace hal
