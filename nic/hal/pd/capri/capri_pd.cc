// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// These functions act as a pass through for CAPRI access by PI.


#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_qstate.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
#include "nic/hal/pd/capri/capri_barco_asym_apis.hpp"
#include "nic/hal/pd/capri/capri_barco_sym_apis.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"

namespace hal {
namespace pd {

hal_ret_t
pd_get_start_offset (pd_get_start_offset_args_t *args)
{
    args->offset = get_start_offset(args->reg_name);
    return HAL_RET_OK;
}

hal_ret_t
pd_get_size_kb (pd_get_size_kb_args_t *args)
{
    args->size = get_size_kb(args->reg_name);
    return HAL_RET_OK;
}

hal_ret_t
pd_push_qstate_to_capri (pd_push_qstate_to_capri_args_t *args)
{
    push_qstate_to_capri(args->qstate, args->cos);
    return HAL_RET_OK;
}

hal_ret_t
pd_clear_qstate (pd_clear_qstate_args_t *args) 
{
    clear_qstate(args->qstate);
    return HAL_RET_OK;
}

hal_ret_t
pd_read_qstate (pd_read_qstate_args_t *args)
{
    read_qstate(args->q_addr, args->buf, args->q_size);
    return HAL_RET_OK;
}

hal_ret_t
pd_write_qstate (pd_write_qstate_args_t *args)
{
    write_qstate(args->q_addr, args->buf, args->q_size);
    return HAL_RET_OK;
}

hal_ret_t
pd_get_pc_offset (pd_get_pc_offset_args_t *args)
{
    int32_t ret = get_pc_offset(args->handle, args->prog_name, 
                                args->label, args->offset);

    HAL_ASSERT(ret == 0);
    return HAL_RET_OK;

}

hal_ret_t
pd_capri_hbm_read_mem (pd_capri_hbm_read_mem_args_t *args)
{
    capri_hbm_read_mem(args->addr, args->buf, args->size);
    return HAL_RET_OK;
}

hal_ret_t
pd_capri_hbm_write_mem (pd_capri_hbm_write_mem_args_t *args)
{
    capri_hbm_write_mem(args->addr, args->buf, args->size);
    return HAL_RET_OK;
}

hal_ret_t
pd_capri_program_label_to_offset (pd_capri_program_label_to_offset_args_t *args)
{
    capri_program_label_to_offset(args->handle, args->prog_name,
                                  args->label_name, args->offset);
    return HAL_RET_OK;
}

hal_ret_t
pd_capri_pxb_cfg_lif_bdf (pd_capri_pxb_cfg_lif_bdf_args_t *args)
{
    return capri_pxb_cfg_lif_bdf(args->lif, args->bdf);
}

hal_ret_t
pd_capri_program_to_base_addr (pd_capri_program_to_base_addr_args_t *args)
{
    return (hal_ret_t)capri_program_to_base_addr(args->handle, 
                                      args->prog_name, args->base_addr);
}

hal_ret_t
pd_get_opaque_tag_addr (pd_get_opaque_tag_addr_args_t *args)
{
    return get_opaque_tag_addr(args->ring_type, args->addr);
}

hal_ret_t
pd_capri_barco_asym_req_descr_get (pd_capri_barco_asym_req_descr_get_args_t *args)
{
    return capri_barco_asym_req_descr_get(args->slot_index,  
                                          args->asym_req_descr);
}

hal_ret_t
pd_capri_barco_symm_req_descr_get (pd_capri_barco_symm_req_descr_get_args_t *args)
{
    return capri_barco_symm_req_descr_get(args->ring_type,
                                          args->slot_index,
                                          args->symm_req_descr);
}

hal_ret_t
pd_capri_barco_ring_meta_get (pd_capri_barco_ring_meta_get_args_t *args)
{
    return capri_barco_ring_meta_get(args->ring_type, 
                                     args->pi, args->ci);
}

hal_ret_t
pd_capri_barco_asym_ecc_point_mul_p256 (pd_capri_barco_asym_ecc_point_mul_p256_args_t *args)
{
    return capri_barco_asym_ecc_point_mul_p256(args->p, 
                                               args->n,
                                               args->xg,
                                               args->yg,
                                               args->a,
                                               args->b,
                                               args->x1, 
                                               args->y1,
                                               args->k,
                                               args->x3,
                                               args->y3);
}

hal_ret_t
pd_capri_barco_asym_ecdsa_p256_setup_private_key(pd_capri_barco_asym_ecdsa_p256_setup_private_key_args_s *args)
{
    return capri_barco_asym_ecdsa_p256_setup_private_key(args->p,
                                               args->n,
                                               args->xg,
                                               args->yg,
                                               args->a,
                                               args->b,
                                               args->da,
                                               args->key_idx);
}

hal_ret_t
pd_capri_barco_asym_ecdsa_p256_sig_gen (pd_capri_barco_asym_ecdsa_p256_sig_gen_args_s *args)
{
    return capri_barco_asym_ecdsa_p256_sig_gen(args->key_idx,
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
                                               args->s);
}

hal_ret_t
pd_capri_barco_asym_ecdsa_p256_sig_verify (pd_capri_barco_asym_ecdsa_p256_sig_verify_args_t *args)
{
    return capri_barco_asym_ecdsa_p256_sig_verify(args->p,
                                                  args->n,
                                                  args->xg,
                                                  args->yg,
                                                  args->a,
                                                  args->b,
                                                  args->xq,
                                                  args->yq,
                                                  args->r,
                                                  args->s,
                                                  args->h);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_encrypt (pd_capri_barco_asym_rsa2k_encrypt_args_t *args)
{
    return capri_barco_asym_rsa2k_encrypt(args->n,
                                          args->e,
                                          args->m,
                                          args->c);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_decrypt (pd_capri_barco_asym_rsa2k_decrypt_args_t *args)
{
    return capri_barco_asym_rsa2k_decrypt(args->n,
                                          args->d,
                                          args->c,
                                          args->m);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_crt_decrypt (pd_capri_barco_asym_rsa2k_crt_decrypt_args_t *args)
{
    return capri_barco_asym_rsa2k_crt_decrypt(args->p,
                                              args->q,
                                              args->dp,
                                              args->dq,
                                              args->qinv,
                                              args->c,
                                              args->m);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_setup_private_key (pd_capri_barco_asym_rsa2k_setup_private_key_args_t *args)
{
    return capri_barco_asym_rsa2k_setup_private_key(
                                          args->n, 
                                          args->d,
                                          args->key_idx);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_sig_gen (pd_capri_barco_asym_rsa2k_sig_gen_args_t *args)
{
    return capri_barco_asym_rsa2k_sig_gen(args->key_idx,
                                          args->n, 
                                          args->d,
                                          args->h,
                                          args->s);
}

hal_ret_t
pd_capri_barco_asym_rsa2k_sig_verify (pd_capri_barco_asym_rsa2k_sig_verify_args_t *args)
{
    return capri_barco_asym_rsa2k_sig_verify(args->n,
                                             args->e,
                                             args->h,
                                             args->s);
}

hal_ret_t
pd_capri_barco_sym_hash_process_request (pd_capri_barco_sym_hash_process_request_args_t *args)
{
    return capri_barco_sym_hash_process_request(args->hash_type, 
                                                args->generate,
                                                args->key,
                                                args->key_len,
                                                args->data,
                                                args->data_len,
                                                args->output_digest,
                                                args->digest_len);
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

    HAL_TRACE_DEBUG ("{0:s} INGRESS: stage {1:d} mpu {2:d} base_addr 0x{3:x}",
                     __FUNCTION__, stage_id, mpu, base_addr);

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
    cap0.sgi.mpu[stage_id].trace[mpu].buf_size(buf_size);
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

    HAL_TRACE_DEBUG ("{0:s} EGRESS: stage {1:d} mpu {2:d} base_addr 0x{3:x}",
                     __FUNCTION__, stage_id, mpu, base_addr);

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
    cap0.sge.mpu[stage_id].trace[mpu].buf_size(buf_size);
    cap0.sge.mpu[stage_id].trace[mpu].enable(enable);
    cap0.sge.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.sge.mpu[stage_id].trace[mpu].write();

    return HAL_RET_OK;
}

// Enable MPU tracing on p4 ingress and p4 egress
// TODO deprecate later
static hal_ret_t
capri_mpu_trace_enable(void)
{
    uint64_t base_addr = get_start_offset("mpu-trace");
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; ++j) {
            HAL_TRACE_DEBUG ("{0:s} INGRESS: stage {1:d} mpu {2:d} base_addr 0x{3:x}",
                             __FUNCTION__, i, j, base_addr);

            cap0.sgi.mpu[i].trace[j].read();
            cap0.sgi.mpu[i].trace[j].base_addr(base_addr >> 6);
            cap0.sgi.mpu[i].trace[j].buf_size(5);
            cap0.sgi.mpu[i].trace[j].wrap(1);
            cap0.sgi.mpu[i].trace[j].table_and_key(1);
            cap0.sgi.mpu[i].trace[j].instructions(0);
            cap0.sgi.mpu[i].trace[j].enable(1);
            cap0.sgi.mpu[i].trace[j].write();

            base_addr += 2048;
        }
    }

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; ++j) {
            HAL_TRACE_DEBUG ("{0:s} EGRESS: stage {1:d} mpu {2:d} base_addr 0x{3:x}",
                             __FUNCTION__, i, j, base_addr);


            cap0.sge.mpu[i].trace[j].read();
            cap0.sge.mpu[i].trace[j].base_addr(base_addr >> 6);
            cap0.sge.mpu[i].trace[j].buf_size(5);
            cap0.sge.mpu[i].trace[j].wrap(1);
            cap0.sge.mpu[i].trace[j].table_and_key(1);
            cap0.sge.mpu[i].trace[j].instructions(0);
            cap0.sge.mpu[i].trace[j].enable(1);
            cap0.sge.mpu[i].trace[j].write();

            base_addr += 2048;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_mpu_trace_enable(pd_mpu_trace_enable_args_t *args)
{
    uint64_t base_addr = get_start_offset("mpu-trace");

    if (args->base_addr == 0) {
        base_addr =
            base_addr +
            (args->stage_id * args->max_mpu_per_stage * args->mpu_trace_size) +
            (args->mpu * args->mpu_trace_size);
    } else {
        base_addr = args->base_addr;
    }

    switch (args->pipeline_type) {
    case MPU_TRACE_PIPELINE_P4_INGRESS:
        return capri_p4_ingress_mpu_trace_enable(args->stage_id,
                                                 args->mpu,
                                                 args->enable,
                                                 args->trace_enable,
                                                 args->phv_debug,
                                                 args->phv_error,
                                                 args->watch_pc,
                                                 base_addr,
                                                 args->table_key,
                                                 args->instructions,
                                                 args->wrap,
                                                 args->reset,
                                                 args->buf_size);

    case MPU_TRACE_PIPELINE_P4_EGRESS:
        return capri_p4_egress_mpu_trace_enable(args->stage_id,
                                                args->mpu,
                                                args->enable,
                                                args->trace_enable,
                                                args->phv_debug,
                                                args->phv_error,
                                                args->watch_pc,
                                                base_addr,
                                                args->table_key,
                                                args->instructions,
                                                args->wrap,
                                                args->reset,
                                                args->buf_size);

    default:
        return capri_mpu_trace_enable();
    }

    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal
