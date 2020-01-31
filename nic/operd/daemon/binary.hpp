#ifndef __OPERD_DAEMON_BINARY_H__
#define __OPERD_DAEMON_BINARY_H__

#include <memory>
#include <string>

#include "lib/operd/operd.hpp"
#include "output.hpp"

class binary : public output {
public:
    static std::shared_ptr<binary> factory(std::string path);
    binary(std::string path);
    ~binary();
    void handle(sdk::operd::log_ptr entry) override;
private:
    int fork_exec(void);
private:
    std::string path_;
    int fd_;
    pid_t pid_;
};
typedef std::shared_ptr<binary> binary_ptr;
    
#endif
