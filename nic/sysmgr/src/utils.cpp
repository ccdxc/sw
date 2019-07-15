#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <zlib.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "nic/utils/penlog/lib/penlog.hpp"
#include "nic/utils/penlog/lib/null_logger.hpp"
#include "gen/proto/device.pb.h"

#define CHUNK_SIZE (64 * 1024)

const pid_t mypid = getpid();
std::string log_location = "/var/log/sysmgr";
penlog::LoggerPtr logger = std::make_shared<penlog::NullLogger>();

namespace pt = boost::property_tree;

void redirect(const std::string &filename, int fd)
{
    int file_fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    
    close(fd);

    dup2(file_fd, fd);
    
    close(file_fd);
}

// Redirect stdout and stderr
void redirect_stds(const std::string &name, pid_t pid)
{
    redirect(log_location + "/" + name + "." + std::to_string(pid) +
        ".out" + ".log", 1);
    redirect(log_location + "/" + name + "." + std::to_string(pid) +
        ".err" + ".log", 2);
}

std::string get_logname_for_process(std::string name, int pid, std::string suffix)
{
    return log_location + "/" + name + "." + std::to_string(pid) +
        "." + suffix + ".log";
}

void exists_or_mkdir(const char *dir)
{
    struct stat sb;
    if (stat(dir, &sb) == 0) {
        if (!S_ISDIR(sb.st_mode)) {
            logger->critical("%s is not a directory");
            exit(-1);
        }
        return;
    }
    
    mkdir(dir, S_IRWXU);
    logger->debug("Creating directory {}", dir);
}

void mkdirs(const char *dir)
{
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;
    struct stat sb;

    // if file exists bail out
    if (stat(dir, &sb) == 0) {
        if (!S_ISDIR(sb.st_mode)) {
            logger->error("%s is not a directory");
            exit(-1);
        }
        return;
    }
    
    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);

    if(tmp[len - 1] == '/')
    {
        tmp[len - 1] = 0;
    }
                
    for(p = tmp + 1; *p; p++)
    {
        if(*p == '/')
        {
            *p = 0;
            exists_or_mkdir(tmp);
            *p = '/';
        }
    }
    exists_or_mkdir(tmp);
}

void replace_fd(int from, int to)
{
    close(to);

    dup2(from, to);

    close(from);
}

void exec_command(const std::string &command)
{
    // split the command to an array of string(tokens)
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        back_inserter(tokens));

    // convert them to c_strings
    std::vector<const char *> c_strings;
    for (auto &&s : tokens)
    {
        c_strings.push_back(s.c_str());
    }
    c_strings.push_back(NULL);

    execvp(c_strings[0], (char *const *)&c_strings[0]);
    fprintf(stderr, "exec(%s) failed: %s", c_strings[0], strerror(errno));
    exit(-1); // the only way to get here is if exec failed, exit
}

void launch(const std::string &name, const std::string &command,
    unsigned long cpu_affinity, process_t *new_process)
{
    pid_t pid;
    int outfds[2];
    int errfds[2];

    pipe(outfds);
    pipe(errfds);

    pid = fork();

    if (pid == -1)
    {
        logger->error("Fork failed: {}", strerror(errno));
        exit(1);
    }
    else if (pid == 0)
    {
        // replace the stdout with the "output" side of the "stdout" pipe
        replace_fd(outfds[1], 0);
        // for the child we close the "input" side of the pipe
        close(outfds[0]);
        // replace the stderr with the "output" side of the "stderr" pipe
        replace_fd(errfds[1], 1);
        // for the child we close the "input" side of the pipe
        close(errfds[0]);
        cpulock(cpu_affinity);
        exec_command(command);
    }

    // On the parent side we close the "output" side of the pipes
    close(outfds[1]);
    close(errfds[1]);

    logger->info("Fork success. Child pid: {}", pid);

    new_process->pid = pid;
    new_process->stdout = outfds[0];
    new_process->stderr = errfds[0];
}

void switch_root()
{
    //kill(-mypid, SIGTERM);
    int ret = execlp("/nic/tools/switch_rootfs.sh",
        "/nic/tools/switch_rootfs.sh", "altfw", NULL);
    printf("execlp return value %d", ret);
    fprintf(stderr, "Switch root failed");
    exit(-1);
}

std::string parse_status(int status)
{
    if (WIFEXITED(status))
    {
        return boost::str(boost::format("Exited normally with code: %1%") %
            WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        return boost::str(boost::format("Exited due to signal: %1%") %
            WTERMSIG(status));
    }
    else
    {
        return boost::str(boost::format("Exited with unparsed status: %1%") %
            status);
    }    
}

std::string get_filename(const std::string &name, const std::string &suffix,
    pid_t pid)
{
    char timestring[PATH_MAX];
    char filename[PATH_MAX];
    time_t current_time = time(NULL);

    strftime(timestring, PATH_MAX, "%Y%m%d%H%M%S", gmtime(&current_time));
    
    snprintf(filename, PATH_MAX, "/data/core/%s_%i_%s_%s.gz", name.c_str(), pid,
        suffix.c_str(), timestring);

    return std::string(filename);
}

void copy_std(const std::string &name, pid_t pid, const std::string &suffix)
{
    std::string src_filename = log_location + "/" + name + "." +
        std::to_string(pid) + "." + suffix + ".log";
    std::string dest_filename = get_filename(name, suffix, pid);

    logger->debug("Copying {} to {}", src_filename, dest_filename);

    int in = open(src_filename.c_str(), O_RDONLY);
    if (in < 0)
    {
        logger->warn("Couldn't open {}", src_filename);
        return;
    }
    gzFile out = gzopen(dest_filename.c_str(), "w");
    while (true) {
        char buffer[CHUNK_SIZE];
        ssize_t n = read(in, buffer, CHUNK_SIZE);
        if (n <= 0) 
        {
            logger->debug("EOF");
            gzclose(out);
            close(in);
            return;
        }
        gzwrite(out, buffer, n);
        logger->debug("Wrote {} bytes", n);
    }
}

void save_stdout_stderr(const std::string &name, pid_t pid)
{
    copy_std(name, pid, "out");
    copy_std(name, pid, "err");
}

// Set the cpu_affinity
void cpulock(unsigned long cpu_affinity)
{
    cpu_set_t set;
    int rc;
    pid_t pid;

    CPU_ZERO(&set);
    for (size_t i = 0; i < sizeof(cpu_affinity) * 8; i++)
    {
        if (cpu_affinity & (1 << i))
        {
            CPU_SET(i, &set);
            logger->debug("Setting affinity to cpu {}", i);
        }
    }
    pid = getpid();

    rc = sched_setaffinity(pid, sizeof(set), &set);
    if (rc == -1)
    {
        logger->error("Failed(%d) to set the affinity for pid {}", errno, pid);
    }
}

std::string get_main_config_file()
{
    pt::ptree ptree;
    std::string fwd_mode;
    std::string feature;

    if (access(DEVICE_JSON, R_OK) < 0)
    {
        return DEFAULT_SYSMGR_JSON;
    }

    pt::read_json(DEVICE_JSON, ptree);

    fwd_mode = ptree.get<std::string>("forwarding-mode",
        "FORWARDING_MODE_CLASSIC");
    if (fwd_mode == "FORWARDING_MODE_CLASSIC")
    {
        fwd_mode = "classic";
    }
    else if (fwd_mode == "FORWARDING_MODE_HOSTPIN")
    {
        fwd_mode = "hostpin";
    }
    else if (fwd_mode == "FORWARDING_MODE_SWITCH")
    {
        fwd_mode = "switch";
    }
    else
    {
        logger->error("Unknown forwarding mode '{}'", fwd_mode);
        return DEFAULT_SYSMGR_JSON;
    }

    int feature_profile = ptree.get<int>("feature-profile", 
                                         device::FEATURE_PROFILE_CLASSIC_DEFAULT);
    if (feature_profile == device::FEATURE_PROFILE_CLASSIC_DEFAULT) {
        feature = "default";
    } else if (feature_profile == device::FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE) {
        feature = "eth-dev-scale";
    } else {
        logger->error("Unknown feature profile '{}'", feature);
        return DEFAULT_SYSMGR_JSON;
    }

#if 0
    feature = ptree.get<std::string>("feature-profile", "classic-default");
    if (feature == "classic-default")
    {
        feature = "default";
    }
    else if (feature == "classic-eth-dev-scale")
    {
        feature = "eth-dev-scale";
    }
    else
    {
        logger->error("Unknown feature profile '{}'", feature);
        return DEFAULT_SYSMGR_JSON;
    }
#endif

    return "/nic/conf/sysmgr-" + fwd_mode + "-" + feature + ".json";
}
