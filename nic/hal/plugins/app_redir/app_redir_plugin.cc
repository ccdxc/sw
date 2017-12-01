#include "nic/hal/plugins/app_redir/app_redir_plugin.hpp"
#include "nic/include/hal_cfg.hpp"
#include "app_redir_scanner.hpp"

namespace hal {
namespace app_redir {

const std::string FTE_FEATURE_APP_REDIR_MISS("pensando.io/app-redir:app-redir-miss");
const std::string FTE_FEATURE_APP_REDIR("pensando.io/app-redir:app-redir");
const std::string FTE_FEATURE_APP_REDIR_APPID("pensando.io/app-redir:app-redir-appid");
const std::string FTE_FEATURE_APP_REDIR_FINI("pensando.io/app-redir:app-redir-fini");

extern "C" {

void app_redir_init() {
    fte::register_feature(FTE_FEATURE_APP_REDIR_MISS, app_redir_miss_exec);
    fte::register_feature(FTE_FEATURE_APP_REDIR, app_redir_exec);
    fte::register_feature(FTE_FEATURE_APP_REDIR_APPID, appid_exec);
    fte::register_feature(FTE_FEATURE_APP_REDIR_FINI, app_redir_exec_fini);
}

void app_redir_exit() {
    scanner_cleanup(hal::HAL_THREAD_ID_CFG);
}

void app_redir_thread_init(int tid) {
    hal::utils::thread* t = hal::utils::thread::current_thread();
    if (t) {
        // Must be run in the correct thread
        assert((int) t->thread_id() == tid);
    }
    scanner_init(tid);
}

void app_redir_thread_exit(int tid) {
    hal::utils::thread* t = hal::utils::thread::current_thread();
    if (t) {
        // Must be run in the correct thread
        assert((int) t->thread_id() == tid);
    }
    scanner_cleanup(tid);
}

} // extern "C"


}  // namespace app_redir
}  // namespace hal
