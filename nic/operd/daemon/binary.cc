#include <assert.h>
#include <fcntl.h>
#include <iterator>
#include <memory>
#include <signal.h>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "lib/operd/operd.hpp"
#include "output.hpp"
#include "binary.hpp"

static void
exec_command (const std::string &command)
{
    fprintf(stderr, "Executing %s\n", command.c_str());
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
    fprintf(stderr, "exec(%s) failed: %s\n", c_strings[0], strerror(errno));
    exit(-1); // the only way to get here is if exec failed, exit
}

binary::binary(std::string path) {
    this->pid_ = 0;
    this->path_ = path;
    this->fd_ = -1;
}

binary::~binary() {
    if (this->pid_ != 0) {
        kill(this->pid_, SIGQUIT);
    }
    if (this->fd_ != -1) {
        close(this->fd_);
    }
}

binary_ptr
binary::factory(std::string path) {
    binary_ptr bin;
    int rc;

    bin = std::make_shared<binary>(path);
    
    rc = bin->fork_exec();
    if (rc == -1) {
        return nullptr;
    }

    return bin;
}

int
binary::fork_exec(void) {
    pid_t pid;
    int fds[2];
    int rc;

    // 0 is the "read" where the new binary will read from
    // 1 is the "write" where the daemon will write to
    rc = pipe(fds);
    if (rc == -1) {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
    }

    if (pid == 0) {
        // close the "write" side of the pipe
        close(fds[1]);
        // close the "stdin"
        close(STDIN_FILENO);
        // and replace it with the fd
        dup2(fds[0], STDIN_FILENO);
        // finally start the command
        exec_command(this->path_);
        // we never return after the above function
    }

    this->pid_ = pid;
    this->fd_ = fds[1];
    // close the "read" side of the pipe, we don't need it in daemon
    close(fds[0]);

    return 0;
}

void
binary::handle(sdk::operd::log_ptr entry) {
    ssize_t n;
    uint64_t data_length;
    int64_t timestamp;

    if (this->fd_ == -1) {
        return;
    }

    data_length = entry->data_length();
    n = write(this->fd_, &data_length, sizeof(data_length));
    if (n != sizeof(data_length)) {
        fprintf(stderr, "failed writing data_length to %s\n",
                this->path_.c_str());
        close(this->fd_);
        this->fd_ = -1;
        return;
    }

    timestamp = static_cast<int64_t>(entry->timestamp()->tv_sec);
    n = write(this->fd_, &timestamp, sizeof(timestamp));
    if (n != sizeof(timestamp)) {
        fprintf(stderr, "failed writing timestamp to %s\n",
                this->path_.c_str());
        close(this->fd_);
        this->fd_ = -1;
        return;
    }
    
    n = write(this->fd_, entry->data(), entry->data_length());
    if (n == -1) {
        fprintf(stderr, "failed to write to %s (%s)\n", this->path_.c_str(),
                strerror(errno));
        close(this->fd_);
        this->fd_ = -1;
        return;
    }
    if (n < 0 || (size_t)n != entry->data_length()) {
        fprintf(stderr, "write mismatch to %s\n", this->path_.c_str());
        return;
    }
}
