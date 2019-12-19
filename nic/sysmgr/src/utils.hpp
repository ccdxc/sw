#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>

#include "bus_api.hpp"
#include "events_api.hpp"
#include "pipedio.hpp"

#include "nic/utils/penlog/lib/penlog.hpp"

#define DEVICE_JSON "/sysconfig/config0/device.conf"
#define DEFAULT_SYSMGR_JSON "/nic/conf/sysmgr.json"

extern const pid_t       mypid;
extern std::string       log_location;

typedef struct process_ {
    pid_t pid;
    int   stdout;
    int   stderr;
} process_t;

extern SysmgrBusPtr g_bus;
extern SysmgrEventsPtr g_events;

extern void        exec_command(const std::string &command);
extern void        exists_or_mkdir(const char *dir);
extern void        launch(const std::string &name, const std::string &command,
                          unsigned long cpu_affinity, double mem_limit,
                          process_t *new_process);
extern void        mkdirs(const char *dir);
extern std::string parse_status(int status);
extern void        redirect(const std::string &filename, int fd);
extern void        redirect_stds(const std::string &name, pid_t pid);
extern void        switch_root();
extern void        save_stdout_stderr(const std::string &name, pid_t pid);
extern void        cpulock(unsigned long cpu_affinity);
extern std::string get_logname_for_process(std::string name, int pid,
                                           std::string suffix);
extern std::string get_main_config_file();
extern void        run_debug(pid_t crashed_pid);

#endif // __UTILS_HPP__
