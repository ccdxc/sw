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
// #include "nic/hal/pd/capri/capri_hbm.hpp"
// #include "nic/hal/pd/capri/capri_pxb_pcie.hpp"

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

namespace hal {

void GetProgramAddress(const internal::ProgramAddressReq &req,
                       internal::ProgramAddressResp *resp) {
    uint64_t addr;
    if (req.resolve_label()) {
        pd::pd_capri_program_label_to_offset_args_t args = {0};
        args.handle = req.handle().c_str();
        args.prog_name = (char *)req.prog_name().c_str();
        args.label_name = (char *)req.label().c_str();
        args.offset = &addr;
        hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, 
                                  (void *)&args);
        if (ret != HAL_RET_OK)
            resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        else
            resp->set_addr(addr);
    } else {
        pd::pd_capri_program_to_base_addr_args_t base_args = {0};
        base_args.handle = req.handle().c_str();
        base_args.prog_name = (char *)req.prog_name().c_str();
        base_args.base_addr = &addr;
        hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_TO_BASE_ADDR, 
                                  (void *)&base_args);
        if (ret != HAL_RET_OK)
            resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        else
            resp->set_addr(addr);
    }
}

void AllocHbmAddress(const internal::HbmAddressReq &req,
                       internal::HbmAddressResp *resp) {
    pd::pd_get_start_offset_args_t args;
    args.reg_name = req.handle().c_str();
    pd::hal_pd_call(pd::PD_FUNC_ID_GET_START_OFFSET, (void *)&args);
    uint64_t addr = args.offset;

    pd::pd_get_size_kb_args_t size_args;
    size_args.reg_name = req.handle().c_str();
    pd::hal_pd_call(pd::PD_FUNC_ID_GET_REG_SIZE, (void *)&size_args);
    uint32_t size = size_args.size;


    HAL_TRACE_DEBUG("pi-internal:{}: Allocated HBM Address {:#x} Size {}",
                    __FUNCTION__, addr, size);
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
   pd::pd_capri_pxb_cfg_lif_bdf_args_t args = {0};
   args.lif = req.lif();
   args.bdf = req.bdf();
   int ret = (int)pd::hal_pd_call(pd::PD_FUNC_ID_PXB_CFG_LIF_BDF, 
                                  (void *)&args);

   resp->set_lif(req.lif());
   resp->set_bdf(req.bdf());
   resp->set_status(ret);
}
}    // namespace hal
