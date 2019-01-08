//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// Implementation of CAPRI LIF Manager.
//-----------------------------------------------------------------------------

#ifndef __LIF_MANAGER_HPP__
#define __LIF_MANAGER_HPP__

#include <memory>
#include <map>
#include "platform/utils/mpartition.hpp"
#include "platform/utils/lif_manager_base.hpp"
#include "platform/utils/program.hpp"

using namespace sdk::platform::utils;

namespace sdk {
namespace platform {
namespace capri {

class LIFManager : public LIFManagerBase {
public:
    static LIFManager *factory(mpartition *mp,
                               program_info *pinfo,
                               const char *kHBMLabel);
    static void destroy(LIFManager *lm);
    void set_program_info(program_info *pinfo);
    virtual int32_t GetPCOffset(const char *handle, const char *prog_name,
                                const char *label, uint8_t *offset);
    class mpartition *get_mpartition(void) { return mp_; }
protected:
    virtual int32_t InitLIFQStateImpl(LIFQState *qstate, int cos);
    virtual int32_t ReadQStateImpl(
            uint64_t q_addr, uint8_t *buf, uint32_t q_size);
    virtual int32_t WriteQStateImpl(
            uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
    virtual void DeleteLIFQStateImpl(LIFQState *qstate);

private:
    LIFManager() {}
    virtual ~LIFManager() {}

    class program_info *pinfo_;
    class mpartition *mp_;
    std::unique_ptr<sdk::lib::BMAllocator> hbm_allocator_;
    uint64_t hbm_base_;

    // Track allocation size which are needed when we
    // free memory.
    std::map<uint32_t, uint32_t> allocation_sizes_;
};

} // namespace capri
} // namespace platform
} // namespace sdk

#endif // _LIF_MANAGER_HPP_

