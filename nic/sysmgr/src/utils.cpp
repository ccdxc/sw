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

#include <ev.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "cgroups.hpp"
#include "log.hpp"

#define CHUNK_SIZE (64 * 1024)

const pid_t mypid = getpid();
std::string log_location = "/var/log/pensando/sysmgr";

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
            glog->critical("%s is not a directory");
            exit(-1);
        }
        return;
    }
    
    mkdir(dir, S_IRWXU);
    glog->debug("Creating directory {}", dir);
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
            glog->error("%s is not a directory");
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

void run_debug(pid_t crashed_pid)
{
    pid_t  pid;
    int    fd;
    char   cmd[256];

    pid = fork();
    if (pid == -1)
    {
        glog->error("Fork failed: {}", strerror(errno));
    }
    else if (pid == 0)
    {
        ev_loop_fork(EV_DEFAULT);
        ev_loop_destroy(EV_DEFAULT);

        // Ignore the stdout and stderr of the script
        fd = open("/dev/null", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        replace_fd(fd, 1);
        replace_fd(fd, 2);
        
        snprintf(cmd, sizeof(cmd), "/nic/tools/sysmgr_debug.sh %u", crashed_pid);
        exec_command(cmd);
    }
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

void close_on_exec(int fd)
{
    int flags;
    int rc;

    flags = fcntl(fd, F_GETFD, 0);
    if (flags == -1)
    {
        glog->error("fcntl F_GETFD: %d", errno);
        return;
    }

    flags |= FD_CLOEXEC;
    rc = fcntl(fd, F_SETFD, flags);
    if (rc == -1)
    {
        glog->error("fcntl F_SETFD: %d", errno);
    }
}

void launch(const std::string &name, const std::string &command,
            unsigned long cpu_affinity, double mem_limit,
            process_t *new_process)
{
    pid_t pid;
    int outfds[2];
    int errfds[2];

    /* 
     * We are using the pipe for the stdout and stderr of the children so we can
     * rotate the files. The "output" side of the pipe is where the child writes
     *
     * The "input" side of the pipe should be closed whenever we do an exec. It
     * is only supposed to be open in the sysmgr and not in the children
     */
    pipe(outfds);
    close_on_exec(outfds[0]);
    pipe(errfds);
    close_on_exec(errfds[0]);

    pid = fork();

    if (pid == -1)
    {
        glog->error("Fork failed: {}", strerror(errno));
        exit(1);
    }
    else if (pid == 0)
    {
        // Destroy the ev loop
        ev_loop_fork(EV_DEFAULT);
        ev_loop_destroy(EV_DEFAULT);
        // replace the stdout with the "output" side of the "stdout" pipe
        replace_fd(outfds[1], 1);
        // replace the stderr with the "output" side of the "stderr" pipe
        replace_fd(errfds[1], 2);
        cpulock(cpu_affinity);
        // add it to the cgroup if there is a mem_limit
        if (mem_limit) {
            cg_add(name.c_str(), getpid());
        }
        exec_command(command);
    }

    // On the parent side we close the "output" side of the pipes
    close(outfds[1]);
    close(errfds[1]);

    glog->info("Fork success. Child pid: {}", pid);

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

    glog->debug("Copying {} to {}", src_filename, dest_filename);

    int in = open(src_filename.c_str(), O_RDONLY);
    if (in < 0)
    {
        glog->warn("Couldn't open {}", src_filename);
        return;
    }
    gzFile out = gzopen(dest_filename.c_str(), "w");
    while (true) {
        char buffer[CHUNK_SIZE];
        ssize_t n = read(in, buffer, CHUNK_SIZE);
        if (n <= 0) 
        {
            glog->debug("EOF");
            gzclose(out);
            close(in);
            return;
        }
        gzwrite(out, buffer, n);
        glog->debug("Wrote {} bytes", n);
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
            glog->debug("Setting affinity to cpu {}", i);
        }
    }
    pid = getpid();

    rc = sched_setaffinity(pid, sizeof(set), &set);
    if (rc == -1)
    {
        glog->error("Failed(%d) to set the affinity for pid {}", errno, pid);
    }
}

