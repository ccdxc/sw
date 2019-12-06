#ifndef __SDK_IPC_SUBSCRIBERS_H__
#define __SDK_IPC_SUBSCRIBERS_H__

#include <vector>

#include <stdint.h>

#include "ipc.hpp"

namespace sdk {
namespace ipc {

class subscribers {
public:
    void set(uint32_t code, uint32_t client);
    void clear(uint32_t code, uint32_t client);
    std::vector<uint32_t> get(uint32_t code);
    static subscribers *instance(void);
private:
    static subscribers *instance_;
    subscribers();
    ~subscribers();
    void shmopen(void);
    uint8_t *subs_;
};

} // namespace sdk
} // namespace ipc

#endif // __SDK_IPC_SUBSCRIBERS_H__
