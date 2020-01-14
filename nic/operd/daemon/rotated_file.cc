#include <assert.h>
#include <fcntl.h>
#include <memory>
#include <stdint.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/operd/operd.hpp"

#include "rotated_file.hpp"
#include "output.hpp"

static char
level_to_c (uint8_t level) {
    switch (level) {
    case 1:
        return 'E';
    case 2:
        return 'W';
    case 3:
        return 'I';
    case 4:
        return 'D';
    default:
        return 'T';
    }
}

rotated_file_ptr
rotated_file::factory(std::string path, uint8_t level, off_t max_size,
                      int max_files) {

    return std::make_shared<rotated_file>(path, level, max_size, max_files);
}


rotated_file::rotated_file(std::string path, uint8_t level, off_t max_size,
                           int max_files) {
    this->level_ = level;
    this->path_ = path;
    this->max_size_ = max_size;
    this->max_files_ = max_files;
    this->fd_ = -1;
    this->current_size_ = 0;
    this->open_file_();
}

rotated_file::~rotated_file() {
    this->close_file_();
}

void
rotated_file::open_file_(void) {
    struct stat stat;
    int rc;
    
    this->fd_ = open(this->path_.c_str(), O_RDWR | O_CREAT, 0666);
    assert(this->fd_ != -1);

    rc = fstat(this->fd_, &stat);
    assert(rc != -1);

    if (stat.st_size != 0) {
        lseek(this->fd_, stat.st_size, 0);
        this->current_size_ = stat.st_size;
    }
}

void
rotated_file::close_file_(void) {
    if (this->fd_ != -1) {
        close(this->fd_);
   } 
}

void
rotated_file::rotate_files_(void) {
    
    std::string old_name;
    std::string new_name;
    int rc;
    
    if (this->current_size_ < this->max_size_) {
        return;
    }

    this->close_file_();

    for (int i = this->max_files_; i > 0; i--) {
        if (i == 1)
            old_name = this->path_;
        else 
            old_name = this->path_ + "." + std::to_string((i - 1));
        new_name = this->path_ + "." + std::to_string(i);
        
        
        if (access(old_name.c_str(), F_OK) != 0) {
            // file not there, skip it
            continue;
        }
        
        rc = rename(old_name.c_str(), new_name.c_str());
        assert(rc != -1);
    }

    this->open_file_();
}

void
rotated_file::handle(sdk::operd::log_ptr entry) {
    char timebuf[256];
    const struct timeval *ts;
    struct tm *ltm;
    int n;

    if (entry->severity() > this->level_) {
        return;
    }

    ts = entry->timestamp();
    ltm = localtime(&ts->tv_sec);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", ltm);
    
    n = dprintf(this->fd_, "%c [%s.%06ld] %i: %s\n",
                level_to_c(entry->severity()), timebuf, ts->tv_usec,
                entry->pid(), entry->data());

    this->current_size_ += n;
    if (this->current_size_ >= this->max_size_) {
        this->rotate_files_();
    }
}
