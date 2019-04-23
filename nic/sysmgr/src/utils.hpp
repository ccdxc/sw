#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>

#include "nic/utils/penlog/lib/penlog.hpp"


extern const pid_t       mypid;
extern std::string       log_location;
extern penlog::LoggerPtr logger;
extern delphi::SdkPtr    delphi_sdk;

extern void        exec_command(const std::string &command);
extern void        exists_or_mkdir(const char *dir);
extern pid_t       launch(const std::string &name, const std::string &command);
extern void        mkdirs(const char *dir);
extern std::string parse_status(int status);
extern void        redirect(const std::string &filename, int fd);
extern void        redirect_stds(const std::string &name, pid_t pid);
extern void        switch_root();
extern void        save_stdout_stderr(const std::string &name, pid_t pid);
extern void        cpulock();

#endif // __UTILS_HPP__
