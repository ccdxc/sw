#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <internal_svc.hpp>
#include <internal.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <utils.hpp>
#include <if_utils.hpp>

int capri_program_to_base_addr(const char *handle,
                               char *prog_name, uint64_t *base_addr);
int capri_program_label_to_offset(const char *handle,
                                  char *prog_name, char *label_name,
                                  uint64_t *offset);

using hal::pd::pd_if_args_t;

namespace hal {

void GetProgramAddress(const internal::ProgramAddressReq &req,
                       internal::ProgramAddressResp *resp) {
    uint64_t addr;
    if (req.resolve_label()) {
        int ret = capri_program_label_to_offset(
            req.handle().c_str(), (char *) req.prog_name().c_str(),
            (char *) req.label().c_str(), &addr);
        if (ret < 0)
            resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        else
            resp->set_addr(addr);
    } else {
        int ret = capri_program_to_base_addr(
            req.handle().c_str(), (char *) req.prog_name().c_str(), &addr);
        if (ret < 0)
            resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        else
            resp->set_addr(addr);
    }
}

}    // namespace hal
