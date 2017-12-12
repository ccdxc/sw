#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
//#include <internal_svc.hpp>
#include "nic/gen/proto/hal/interface.grpc.pb.h"
#include "nic/hal/src/internal.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/if_utils.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"

using intf::Interface;
using intf::LifSpec;
using kh::LifKeyHandle;
using intf::LifRequestMsg;
using intf::LifResponse;
using intf::LifResponseMsg;
using intf::LifDeleteRequestMsg;
using intf::LifDeleteResponseMsg;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::InterfaceSpec;
using intf::InterfaceStatus;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponseMsg;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteResponseMsg;
using intf::InterfaceGetRequest;
using intf::InterfaceGetRequestMsg;
using intf::InterfaceGetResponse;
using intf::InterfaceGetResponseMsg;
using intf::InterfaceL2SegmentRequestMsg;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponseMsg;
using intf::InterfaceL2SegmentResponse;
using intf::GetQStateRequestMsg;
using intf::GetQStateResponseMsg;
using intf::SetQStateRequestMsg;
using intf::SetQStateResponseMsg;
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

void AllocHbmAddress(const internal::HbmAddressReq &req,
                       internal::HbmAddressResp *resp) {
    uint64_t addr = get_start_offset(req.handle().c_str());
    uint32_t size = get_size_kb(req.handle().c_str());
    if (addr == 0) {
        resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        resp->set_size(0);
    } else {
        resp->set_addr(addr);
        resp->set_size(size);
    }
}

void ConfigureLifBdf(const internal::LifBdfReq &req,
                     internal::LifBdfResp *resp) {
   int ret = capri_pxb_cfg_lif_bdf(req.lif(), req.bdf());
   resp->set_lif(req.lif());
   resp->set_bdf(req.bdf());
   resp->set_status(ret);
}
}    // namespace hal
