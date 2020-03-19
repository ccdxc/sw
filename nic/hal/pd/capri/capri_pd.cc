// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// These functions act as a pass through for CAPRI access by PI.


#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "platform/capri/capri_pxb_pcie.hpp"
#include "nic/sdk/platform/capri/capri_barco_rings.hpp"
#include "nic/sdk/platform/capri/capri_barco_asym_apis.hpp"
#include "nic/sdk/platform/capri/capri_barco_sym_apis.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/model/utils/cap_blk_reg_model.h"
#include "platform/capri/capri_quiesce.hpp"
#include "platform/capri/capri_qstate.hpp"
#include "asic/pd/pd.hpp"

using namespace sdk::platform::capri;
using namespace sdk::asic::pd;

namespace hal {
namespace pd {

hal_ret_t
pd_push_qstate_to_capri (pd_func_args_t *pd_func_args)
{
    pd_push_qstate_to_capri_args_t *args = pd_func_args->pd_push_qstate_to_capri;
    push_qstate_to_capri(args->qstate, args->cos);
    return HAL_RET_OK;
}

hal_ret_t
pd_clear_qstate (pd_func_args_t *pd_func_args)
{
    pd_clear_qstate_args_t *args = pd_func_args->pd_clear_qstate;
    clear_qstate(args->qstate);
    return HAL_RET_OK;
}

hal_ret_t
pd_read_qstate (pd_func_args_t *pd_func_args)
{
    pd_read_qstate_args_t *args = pd_func_args->pd_read_qstate;
    read_qstate(args->q_addr, args->buf, args->q_size);
    return HAL_RET_OK;
}

hal_ret_t
pd_get_pc_offset (pd_func_args_t *pd_func_args)
{
    pd_get_pc_offset_args_t *args = pd_func_args->pd_get_pc_offset;
    int32_t ret = get_pc_offset(args->pinfo, args->prog_name,
                                args->label, args->offset);

    SDK_ASSERT(ret == 0);
    return HAL_RET_OK;

}

hal_ret_t
pd_capri_hbm_read_mem (pd_func_args_t *pd_func_args)
{
    pd_capri_hbm_read_mem_args_t *args = pd_func_args->pd_capri_hbm_read_mem;
    sdk::asic::asic_mem_read(args->addr, args->buf, args->size);
    return HAL_RET_OK;
}

hal_ret_t
pd_capri_hbm_write_mem (pd_func_args_t *pd_func_args)
{
    mpartition_region_t *reg = NULL;
    p4plus_cache_action_t action = P4PLUS_CACHE_ACTION_NONE;
    pd_capri_hbm_write_mem_args_t *args = pd_func_args->pd_capri_hbm_write_mem;
    sdk::asic::asic_mem_write(args->addr, (uint8_t *)args->buf, args->size);

    reg = sdk::asic::pd::get_hbm_region_by_address(args->addr);
    SDK_ASSERT(reg != NULL);

    if(is_region_cache_pipe_p4plus_all(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_BOTH;
    } else if (is_region_cache_pipe_p4plus_rxdma(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_RXDMA;
    } else if (is_region_cache_pipe_p4plus_txdma(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_TXDMA;
    }

    if (action != P4PLUS_CACHE_ACTION_NONE) {
        p4plus_invalidate_cache(args->addr, args->size, action);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_capri_program_label_to_offset (pd_func_args_t *pd_func_args)
{
    pd_capri_program_label_to_offset_args_t *args = pd_func_args->pd_capri_program_label_to_offset;
    sdk::p4::p4_program_label_to_offset(args->handle, args->prog_name,
                                  args->label_name, args->offset);
    return HAL_RET_OK;
}

hal_ret_t
pd_capri_pxb_cfg_lif_bdf (pd_func_args_t *pd_func_args)
{
    sdk_ret_t sret;
    pd_capri_pxb_cfg_lif_bdf_args_t *args = pd_func_args->pd_capri_pxb_cfg_lif_bdf;
    sret = capri_pxb_cfg_lif_bdf(args->lif, args->bdf);
    return hal_sdk_ret_to_hal_ret(sret);
}

hal_ret_t
pd_capri_program_to_base_addr (pd_func_args_t *pd_func_args)
{
    pd_capri_program_to_base_addr_args_t *args = pd_func_args->pd_capri_program_to_base_addr;
    if(sdk::p4::p4_program_to_base_addr(args->handle,
                                      args->prog_name, args->base_addr) != 0)
        return HAL_RET_ERR;
    return HAL_RET_OK;
}

hal_ret_t
pd_get_opaque_tag_addr (pd_func_args_t *pd_func_args)
{
    pd_get_opaque_tag_addr_args_t *args = pd_func_args->pd_get_opaque_tag_addr;
    return (get_opaque_tag_addr((barco_rings_t)args->ring_type, args->addr) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_req_descr_get (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_req_descr_get_args_t *args = pd_func_args->pd_capri_barco_asym_req_descr_get;
    return (capri_barco_asym_req_descr_get(args->slot_index,
                                          args->asym_req_descr) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_symm_req_descr_get (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_symm_req_descr_get_args_t *args = pd_func_args->pd_capri_barco_symm_req_descr_get;
    return (capri_barco_symm_req_descr_get((barco_rings_t)args->ring_type,
                                          args->slot_index,
                                          args->symm_req_descr) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_ring_meta_get (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_ring_meta_get_args_t *args = pd_func_args->pd_capri_barco_ring_meta_get;
    return (capri_barco_ring_meta_get((barco_rings_t)args->ring_type,
                                     args->pi, args->ci) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_ring_meta_config_get (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_ring_meta_config_get_args_t *args = pd_func_args->pd_capri_barco_ring_meta_config_get;
    return (capri_barco_get_meta_config_info((barco_rings_t)args->ring_type, args->meta) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_add_pend_req(pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_add_pend_req_args_t *args = pd_func_args->pd_capri_barco_asym_add_pend_req;
    return (capri_barco_asym_add_pend_req(args->hw_id,
                                         args->sw_id) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_poll_pend_req(pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_poll_pend_req_args_t *args = pd_func_args->pd_capri_barco_asym_poll_pend_req;
    return (capri_barco_asym_poll_pend_req(args->batch_size,
                                          args->id_count,
                                          args->ids) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_ecc_point_mul(pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_ecc_point_mul_args_t *args = pd_func_args->pd_capri_barco_asym_ecc_point_mul;
    return (capri_barco_asym_ecc_point_mul(     args->key_size,
                                               args->p,
                                               args->n,
                                               args->xg,
                                               args->yg,
                                               args->a,
                                               args->b,
                                               args->x1,
                                               args->y1,
                                               args->k,
                                               args->x3,
                                               args->y3) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_ecdsa_p256_setup_priv_key(pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_ecdsa_p256_setup_priv_key_args_t *args = pd_func_args->pd_capri_barco_asym_ecdsa_p256_setup_priv_key;
    return (capri_barco_asym_ecdsa_p256_setup_priv_key(args->p,
                                                      args->n,
                                                      args->xg,
                                                      args->yg,
                                                      args->a,
                                                      args->b,
                                                      args->da,
                                                      args->key_idx) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_ecdsa_p256_sig_gen (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_ecdsa_p256_sig_gen_args_t *args = pd_func_args->pd_capri_barco_asym_ecdsa_p256_sig_gen;
    return (capri_barco_asym_ecdsa_p256_sig_gen(args->key_idx,
                                               args->p,
                                               args->n,
                                               args->xg,
                                               args->yg,
                                               args->a,
                                               args->b,
                                               args->da,
                                               args->k,
                                               args->h,
                                               args->r,
                                               args->s,
                                               args->async_en,
                                               args->unique_key) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_ecdsa_p256_sig_verify (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_ecdsa_p256_sig_verify_args_t *args = pd_func_args->pd_capri_barco_asym_ecdsa_p256_sig_verify;
    return (capri_barco_asym_ecdsa_p256_sig_verify(args->p,
                                                  args->n,
                                                  args->xg,
                                                  args->yg,
                                                  args->a,
                                                  args->b,
                                                  args->xq,
                                                  args->yq,
                                                  args->r,
                                                  args->s,
                                                  args->h,
                                                  args->async_en,
                                                  args->unique_key) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_encrypt (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_encrypt_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_encrypt;
    return (capri_barco_asym_rsa2k_encrypt(args->n,
                                          args->e,
                                          args->m,
                                          args->c,
                                          args->async_en,
                                          args->unique_key) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa_encrypt (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa_encrypt_args_t *args = pd_func_args->pd_capri_barco_asym_rsa_encrypt;
    return (capri_barco_asym_rsa_encrypt(args->key_size,
                                          args->n,
                                          args->e,
                                          args->m,
                                          args->c,
                                          args->async_en,
                                          args->unique_key) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_decrypt (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_decrypt_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_decrypt;
    return (capri_barco_asym_rsa2k_decrypt(args->n,
                                          args->d,
                                          args->c,
                                          args->m) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_crt_decrypt (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_crt_decrypt_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_crt_decrypt;
    return (capri_barco_asym_rsa2k_crt_decrypt(args->key_idx,
                                              args->p,
                                              args->q,
                                              args->dp,
                                              args->dq,
                                              args->qinv,
                                              args->c,
                                              args->m,
                                              args->async_en,
                                              args->unique_key) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_setup_sig_gen_priv_key (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_setup_sig_gen_priv_key_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_setup_sig_gen_priv_key;
    return (capri_barco_asym_rsa2k_setup_sig_gen_priv_key(args->n,
                                                         args->d,
                                                         args->key_idx) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_crt_setup_decrypt_priv_key(pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_crt_setup_decrypt_priv_key_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_crt_setup_decrypt_priv_key;
    return (capri_barco_asym_rsa2k_crt_setup_decrypt_priv_key(args->p,
                                                             args->q,
                                                             args->dp,
                                                             args->dq,
                                                             args->qinv,
                                                             args->key_idx) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa_setup_priv_key(pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa_setup_priv_key_args_t *args = pd_func_args->pd_capri_barco_asym_rsa_setup_priv_key;
    return (capri_barco_asym_rsa_setup_priv_key(args->key_size,
                                                args->n,
                                                args->d,
                                                args->key_idx) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_sig_gen (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_sig_gen_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_sig_gen;
    return (capri_barco_asym_rsa2k_sig_gen(args->key_idx,
                                          args->n,
                                          args->d,
                                          args->h,
                                          args->s,
                                          args->async_en,
                                          args->unique_key) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa_sig_gen (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa_sig_gen_args_t *args = pd_func_args->pd_capri_barco_asym_rsa_sig_gen;
    return (capri_barco_asym_rsa_sig_gen(args->key_size,
                                          args->key_idx,
                                          args->n,
                                          args->d,
                                          args->h,
                                          args->s,
                                          args->async_en,
                                          args->unique_key) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_fips_rsa_sig_gen (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_fips_rsa_sig_gen_args_t *args = pd_func_args->pd_capri_barco_asym_fips_rsa_sig_gen;
    return (capri_barco_asym_fips_rsa_sig_gen(args->key_size,
                                          args->key_idx,
                                          args->n,
                                          args->e,
                                          args->msg,
                                          args->msg_len,
                                          args->s,
                                          (hash_type_t)args->hash_type,
                                          (rsa_signature_scheme_t)args->sig_scheme,
                                          args->async_en,
                                          args->unique_key) ==
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_fips_rsa_sig_verify (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_fips_rsa_sig_verify_args_t *args = pd_func_args->pd_capri_barco_asym_fips_rsa_sig_verify;
    return (capri_barco_asym_fips_rsa_sig_verify(args->key_size,
                                          args->n,
                                          args->e,
                                          args->msg,
                                          args->msg_len,
                                          args->s,
                                          (hash_type_t)args->hash_type,
                                          (rsa_signature_scheme_t)args->sig_scheme,
                                          args->async_en,
                                          args->unique_key) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_sig_verify (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_asym_rsa2k_sig_verify_args_t *args = pd_func_args->pd_capri_barco_asym_rsa2k_sig_verify;
    return (capri_barco_asym_rsa2k_sig_verify(args->n,
                                             args->e,
                                             args->h,
                                             args->s) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

hal_ret_t
pd_capri_barco_sym_hash_process_request (pd_func_args_t *pd_func_args)
{
    pd_capri_barco_sym_hash_process_request_args_t *args = pd_func_args->pd_capri_barco_sym_hash_process_request;
    return (capri_barco_sym_hash_process_request(args->hash_type,
                                                args->generate,
                                                args->key,
                                                args->key_len,
                                                args->data,
                                                args->data_len,
                                                args->output_digest,
                                                args->digest_len) == 
            SDK_RET_OK ? HAL_RET_OK : HAL_RET_ERR);
}

// Enable MPU tracing on p4 ingress
static hal_ret_t
capri_p4_ingress_mpu_trace_enable(uint32_t stage_id,
                                  uint32_t mpu,
                                  uint8_t  enable,
                                  uint8_t  trace_enable,
                                  uint8_t  phv_debug,
                                  uint8_t  phv_error,
                                  uint64_t watch_pc,
                                  uint64_t base_addr,
                                  uint8_t  table_key,
                                  uint8_t  instructions,
                                  uint8_t  wrap,
                                  uint8_t  reset,
                                  uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    HAL_TRACE_DEBUG ("INGRESS: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    HAL_TRACE_DEBUG ("INGRESS: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    HAL_TRACE_DEBUG ("INGRESS: instructions {:d} buf_size {:d}", instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.sgi.mpu[stage_id].trace[mpu].read();
    cap0.sgi.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.sgi.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.sgi.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.sgi.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.sgi.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.sgi.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.sgi.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.sgi.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.sgi.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.sgi.mpu[stage_id].trace[mpu].rst(reset);
    cap0.sgi.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.sgi.mpu[stage_id].trace[mpu].enable(enable);
    cap0.sgi.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.sgi.mpu[stage_id].trace[mpu].write();

    return HAL_RET_OK;
}

// Enable MPU tracing on p4 egress
static hal_ret_t
capri_p4_egress_mpu_trace_enable(uint32_t stage_id,
                                 uint32_t mpu,
                                 uint8_t  enable,
                                 uint8_t  trace_enable,
                                 uint8_t  phv_debug,
                                 uint8_t  phv_error,
                                 uint64_t watch_pc,
                                 uint64_t base_addr,
                                 uint8_t  table_key,
                                 uint8_t  instructions,
                                 uint8_t  wrap,
                                 uint8_t  reset,
                                 uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    HAL_TRACE_DEBUG ("EGRESS: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    HAL_TRACE_DEBUG ("EGRESS: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    HAL_TRACE_DEBUG ("EGRESS: instructions {:d} buf_size {:d}", instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.sge.mpu[stage_id].trace[mpu].read();
    cap0.sge.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.sge.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.sge.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.sge.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.sge.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.sge.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.sge.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.sge.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.sge.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.sge.mpu[stage_id].trace[mpu].rst(reset);
    cap0.sge.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.sge.mpu[stage_id].trace[mpu].enable(enable);
    cap0.sge.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.sge.mpu[stage_id].trace[mpu].write();

    return HAL_RET_OK;
}

// Enable MPU tracing on p4plus txdma
static hal_ret_t
capri_p4p_txdma_mpu_trace_enable(uint32_t stage_id,
                                 uint32_t mpu,
                                 uint8_t  enable,
                                 uint8_t  trace_enable,
                                 uint8_t  phv_debug,
                                 uint8_t  phv_error,
                                 uint64_t watch_pc,
                                 uint64_t base_addr,
                                 uint8_t  table_key,
                                 uint8_t  instructions,
                                 uint8_t  wrap,
                                 uint8_t  reset,
                                 uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    HAL_TRACE_DEBUG ("TXDMA: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    HAL_TRACE_DEBUG ("TXDMA: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    HAL_TRACE_DEBUG ("TXDMA: instructions {:d} buf_size {:d}", instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.pct.mpu[stage_id].trace[mpu].read();
    cap0.pct.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.pct.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.pct.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.pct.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.pct.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.pct.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.pct.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.pct.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.pct.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.pct.mpu[stage_id].trace[mpu].rst(reset);
    cap0.pct.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.pct.mpu[stage_id].trace[mpu].enable(enable);
    cap0.pct.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.pct.mpu[stage_id].trace[mpu].write();

    return HAL_RET_OK;
}

// Enable MPU tracing on p4plus rxdma
static hal_ret_t
capri_p4p_rxdma_mpu_trace_enable(uint32_t stage_id,
                                 uint32_t mpu,
                                 uint8_t  enable,
                                 uint8_t  trace_enable,
                                 uint8_t  phv_debug,
                                 uint8_t  phv_error,
                                 uint64_t watch_pc,
                                 uint64_t base_addr,
                                 uint8_t  table_key,
                                 uint8_t  instructions,
                                 uint8_t  wrap,
                                 uint8_t  reset,
                                 uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    HAL_TRACE_DEBUG ("RXDMA: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    HAL_TRACE_DEBUG ("RXDMA: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    HAL_TRACE_DEBUG ("RXDMA: instructions {:d} buf_size {:d}", instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.pcr.mpu[stage_id].trace[mpu].read();
    cap0.pcr.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.pcr.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.pcr.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.pcr.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.pcr.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.pcr.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.pcr.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.pcr.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.pcr.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.pcr.mpu[stage_id].trace[mpu].rst(reset);
    cap0.pcr.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.pcr.mpu[stage_id].trace[mpu].enable(enable);
    cap0.pcr.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.pcr.mpu[stage_id].trace[mpu].write();

    return HAL_RET_OK;
}

hal_ret_t
pd_mpu_trace_enable(pd_func_args_t *pd_func_args)
{
    pd_mpu_trace_enable_args_t *args = pd_func_args->pd_mpu_trace_enable;

    switch (args->pipeline_type) {
    case MPU_TRACE_PIPELINE_P4_INGRESS:
        return capri_p4_ingress_mpu_trace_enable(args->stage_id,
                                                 args->mpu,
                                                 args->mpu_trace_info.enable,
                                                 args->mpu_trace_info.trace_enable,
                                                 args->mpu_trace_info.phv_debug,
                                                 args->mpu_trace_info.phv_error,
                                                 args->mpu_trace_info.watch_pc,
                                                 args->mpu_trace_info.base_addr,
                                                 args->mpu_trace_info.table_key,
                                                 args->mpu_trace_info.instructions,
                                                 args->mpu_trace_info.wrap,
                                                 args->mpu_trace_info.reset,
                                                 args->mpu_trace_info.buf_size);

    case MPU_TRACE_PIPELINE_P4_EGRESS:
        return capri_p4_egress_mpu_trace_enable(args->stage_id,
                                                args->mpu,
                                                args->mpu_trace_info.enable,
                                                args->mpu_trace_info.trace_enable,
                                                args->mpu_trace_info.phv_debug,
                                                args->mpu_trace_info.phv_error,
                                                args->mpu_trace_info.watch_pc,
                                                args->mpu_trace_info.base_addr,
                                                args->mpu_trace_info.table_key,
                                                args->mpu_trace_info.instructions,
                                                args->mpu_trace_info.wrap,
                                                args->mpu_trace_info.reset,
                                                args->mpu_trace_info.buf_size);

    case MPU_TRACE_PIPELINE_P4P_RXDMA:
        return capri_p4p_rxdma_mpu_trace_enable(args->stage_id,
                                                args->mpu,
                                                args->mpu_trace_info.enable,
                                                args->mpu_trace_info.trace_enable,
                                                args->mpu_trace_info.phv_debug,
                                                args->mpu_trace_info.phv_error,
                                                args->mpu_trace_info.watch_pc,
                                                args->mpu_trace_info.base_addr,
                                                args->mpu_trace_info.table_key,
                                                args->mpu_trace_info.instructions,
                                                args->mpu_trace_info.wrap,
                                                args->mpu_trace_info.reset,
                                                args->mpu_trace_info.buf_size);
    case MPU_TRACE_PIPELINE_P4P_TXDMA:
        return capri_p4p_txdma_mpu_trace_enable(args->stage_id,
                                                args->mpu,
                                                args->mpu_trace_info.enable,
                                                args->mpu_trace_info.trace_enable,
                                                args->mpu_trace_info.phv_debug,
                                                args->mpu_trace_info.phv_error,
                                                args->mpu_trace_info.watch_pc,
                                                args->mpu_trace_info.base_addr,
                                                args->mpu_trace_info.table_key,
                                                args->mpu_trace_info.instructions,
                                                args->mpu_trace_info.wrap,
                                                args->mpu_trace_info.reset,
                                                args->mpu_trace_info.buf_size);
    default:
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

// Enable MPU tracing on p4 ingress
static hal_ret_t
capri_dpp_int_credit(uint32_t instance, uint32_t value)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    HAL_TRACE_DEBUG ("INGRESS: instance {:d} value {:d}",
                     instance, value);

    cap0.dpp.dpp[instance].int_credit.int_test_set.ptr_credit_ovflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.ptr_credit_undflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.pkt_credit_ovflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.pkt_credit_undflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_credit_ovflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_credit_undflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_hdrfld_vld_ovfl_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_hdrfld_offset_ovfl_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.err_framer_hdrsize_zero_ovfl_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.write();

    return HAL_RET_OK;
}

hal_ret_t
pd_reg_write(pd_func_args_t *pd_func_args)
{
    pd_reg_write_args_t *args = pd_func_args->pd_reg_write;

    switch (args->register_id) {
    case pd_reg_write_type_t::DPP_INT_CREDIT:
        return capri_dpp_int_credit(args->instance,
                                    args->value);
    default:
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

hal_ret_t   pd_quiesce_start(pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = sdk::platform::capri::capri_quiesce_start();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t   pd_quiesce_stop(pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = sdk::platform::capri::capri_quiesce_stop();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

} // namespace pd
} // namespace hal
