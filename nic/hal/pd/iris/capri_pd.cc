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

namespace hal {
namespace pd {

EXTC hal_ret_t
pd_get_start_offset (pd_get_start_offset_args_t *args)
{
    args->offset = get_start_offset(args->reg_name);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_get_size_kb (pd_get_size_kb_args_t *args)
{
    args->size = get_size_kb(args->reg_name);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_push_qstate_to_capri (pd_push_qstate_to_capri_args_t *args)
{
    push_qstate_to_capri(args->qstate);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_clear_qstate (pd_clear_qstate_args_t *args) 
{
    clear_qstate(args->qstate);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_read_qstate (pd_read_qstate_args_t *args)
{
    read_qstate(args->q_addr, args->buf, args->q_size);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_write_qstate (pd_write_qstate_args_t *args)
{
    write_qstate(args->q_addr, args->buf, args->q_size);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_get_pc_offset (pd_get_pc_offset_args_t *args)
{
    int32_t ret = get_pc_offset(args->handle, args->prog_name, 
                                args->label, args->offset);

    HAL_ASSERT(ret == 0);
    return HAL_RET_OK;

}

EXTC hal_ret_t
pd_capri_hbm_read_mem (pd_capri_hbm_read_mem_args_t *args)
{
    capri_hbm_read_mem(args->addr, args->buf, args->size);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_capri_hbm_write_mem (pd_capri_hbm_write_mem_args_t *args)
{
    capri_hbm_write_mem(args->addr, args->buf, args->size);
    return HAL_RET_OK;
}

EXTC hal_ret_t
pd_capri_program_label_to_offset (pd_capri_program_label_to_offset_args_t *args)
{
    capri_program_label_to_offset(args->handle, args->prog_name,
                                  args->label_name, args->offset);
    return HAL_RET_OK;
}

EXTC hal_ret_t
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
pd_capri_barco_asym_ecdsa_p256_sig_gen (pd_capri_barco_asym_ecdsa_p256_sig_gen_args_s *args)
{
    return capri_barco_asym_ecdsa_p256_sig_gen(args->p,
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
pd_capri_barco_asym_rsa2k_sig_gen (pd_capri_barco_asym_rsa2k_sig_gen_args_t *args)
{
    return capri_barco_asym_rsa2k_sig_gen(args->n, 
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

} // namespace pd
} // namespace hal
