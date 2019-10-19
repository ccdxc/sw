#ifndef __LOG_H__
#define __LOG_H__

#include <memory>

#include <spdlog/sinks/file_sinks.h>
#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> glog;

void init_logger(void);

#endif // __LOG_H__
