#include <memory>
#include <string>

#include <spdlog/sinks/file_sinks.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>

#define PENLOG_LOCATION "/var/log"
#define PENLOG_LOGFILE_SIZE (1024 * 1024)
#define PENLOG_ROTATE_COUNT 5

std::shared_ptr<spdlog::logger> glog;

void
init_logger (void) {
    const char *env_location = getenv("PENLOG_LOCATION");
    std::string location;
    if (env_location)
    {
        location = std::string(env_location);
    }
    else
    {
        location = std::string(PENLOG_LOCATION);
    }

    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        location + "/sysmgr.log", PENLOG_LOGFILE_SIZE, PENLOG_ROTATE_COUNT);
    glog = std::make_shared<spdlog::logger>("sysmgr", sink);
    glog->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
    glog->flush_on(spdlog::level::info);
    glog->set_level(spdlog::level::info);
}
