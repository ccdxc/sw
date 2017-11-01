#include "nic/hal/hal.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/pd/capri/capri_barco_res.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
#include "nic/hal/src/crypto_keys.hpp"

namespace hal {
namespace pd {



#define _API_PARAM_DEBUG_

#ifdef _API_PARAM_DEBUG_


#define MAX_LINE_SZ 128
static void hex_dump_trace(char *label, char *buf, uint16_t len)
{
    char            line[MAX_LINE_SZ];
    char            *lineptr;
    uint16_t        idx = 0;
    uint16_t        lineoffset = 0;

    lineptr = &line[0];
    HAL_TRACE_DEBUG("{}:", label);
    for (idx = 0; idx < len; idx++) {

        lineoffset += snprintf(lineptr + lineoffset, (MAX_LINE_SZ - lineoffset - 1),
                "%02hhx ", buf[idx]);

        if (((idx + 1) % 16) == 0) {
            HAL_TRACE_DEBUG("{}", line);
            lineoffset = 0;
        }
    }
    if (lineoffset) {
        HAL_TRACE_DEBUG("{}", line);
    }
}

#define CAPRI_BARCO_API_PARAM_HEXDUMP(label, buf, len) hex_dump_trace(label, buf, len)

#else

#define CAPRI_BARCO_API_PARAM_HEXDUMP(label, buf, len)

#endif /* _API_PARAM_DEBUG_ */


hal_ret_t capri_barco_asym_ecc_point_mul_p256(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *x1, uint8_t *y1,
        uint8_t *k, uint8_t *x3, uint8_t *y3)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint64_t                    ilist_dma_descr_addr = 0, olist_dma_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, olist_mem_addr = 0, curr_ptr = 0;
    uint64_t                    key_param = 0;
    barco_asym_descriptor_t     asym_req_descr;
    barco_asym_dma_descriptor_t ilist_dma_descr, olist_dma_descr;
    int32_t                     ecc_pm_p256_key_idx = -1;
    crypto_asym_key_t           asym_key;
    uint32_t                    req_tag = 0;

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"p", (char *)p, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"a", (char *)a, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"b", (char *)b, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"n", (char *)n, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"xg", (char *)xg, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"yg", (char *)yg, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"x1", (char *)x1, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"y1", (char *)y1, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"k", (char *)k, 32);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &key_param);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to allocate memory for key param");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Allocated memory for key param @ {:x}", key_param); 

    ret = pd_crypto_asym_alloc_key(&ecc_pm_p256_key_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to allocate key descriptor");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Allocated Key Descr @ {:x}", ecc_pm_p256_key_idx); 

    asym_key.key_param_list = key_param; /* Barco does not use key space for ECC Point MUL for now */
    asym_key.command_reg = (CAPRI_BARCO_ASYM_CMD_SWAP_BYTES |
                            CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP(32) |
                            CAPRI_BARCO_ASYM_CMD_ECC_POINT_MUL);
                                
    ret = pd_crypto_asym_write_key(ecc_pm_p256_key_idx, &asym_key);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write key: {}", ecc_pm_p256_key_idx);
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Setup key @ {:x}", ecc_pm_p256_key_idx); 


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &ilist_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to allocate memory for ilist DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Allocated memory for ilist DMA Descr @ {:x}", ilist_dma_descr_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &olist_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to allocate memory for olist DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Allocated memory for olist DMA Descr @ {:x}", olist_dma_descr_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &ilist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to allocate memory for ilist content");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Allocated memory for input mem @ {:x}", ilist_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &olist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to allocate memory for olist content");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("ECC Point Mul P256: Allocated memory for output mem @ {:x}", olist_mem_addr); 

    /* Copy the input to the ilist memory */
    curr_ptr = ilist_mem_addr;
    
    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)p, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param p into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)n, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param n into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)xg, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param xg into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)yg, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param yg into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)a, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param a into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)b, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param b into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)x1, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param x1 into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)y1, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param y1 into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)k, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ECC param k into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    /* Setup ilist DMA descriptor */
    ilist_dma_descr.address = ilist_mem_addr;
    ilist_dma_descr.stop = 1;
    ilist_dma_descr.rsvd0 = 1;
    ilist_dma_descr.next = 0;
    ilist_dma_descr.int_en = 0;
    ilist_dma_descr.discard = 0;
    ilist_dma_descr.realign = 0;
    ilist_dma_descr.cst_addr = 0;
    ilist_dma_descr.length = (curr_ptr - ilist_mem_addr);
    if (capri_hbm_write_mem(ilist_dma_descr_addr, (uint8_t*)&ilist_dma_descr,
                sizeof(ilist_dma_descr))) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write ilist DMA Descr @ {:x}",
                (uint64_t) ilist_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup olist DMA descriptor */
    olist_dma_descr.address = olist_mem_addr;
    olist_dma_descr.stop = 1;
    olist_dma_descr.rsvd0 = 1;
    olist_dma_descr.next = 0;
    olist_dma_descr.int_en = 0;
    olist_dma_descr.discard = 0;
    olist_dma_descr.realign = 0;
    olist_dma_descr.cst_addr = 0;
    olist_dma_descr.length = (2 * 32 ); /* Outputs: x3, y3 */
    if (capri_hbm_write_mem(olist_dma_descr_addr, (uint8_t*)&olist_dma_descr,
                sizeof(olist_dma_descr))) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to write olist DMA Descr @ {:x}",
                (uint64_t) olist_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Asymmetric Request Descriptor */
    asym_req_descr.input_list_addr = ilist_dma_descr_addr;
    asym_req_descr.output_list_addr = olist_dma_descr_addr;
    asym_req_descr.key_descr_idx_lo = (uint8_t) (ecc_pm_p256_key_idx & 0xff);
    asym_req_descr.key_descr_idx_hi = (uint16_t) (ecc_pm_p256_key_idx >> 8);
    asym_req_descr.status_addr = curr_ptr;
    asym_req_descr.opaque_tag_value = 0;
    asym_req_descr.opage_tag_wr_en = 0;
    asym_req_descr.flag_a = 0;
    asym_req_descr.flag_b = 0;

    ret = capri_barco_ring_queue_request(types::BARCO_RING_ASYM, (void *)&asym_req_descr, &req_tag);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to enqueue request");
        ret = HAL_RET_ERR;
        goto cleanup;
    }
    /* Poll for completion */
    while (capri_barco_ring_poll(types::BARCO_RING_ASYM, req_tag) != TRUE) {
        //HAL_TRACE_DEBUG("ECC Point Mul P256: Waiting for Barco completion...");
    }


    /* Copy out the results */
    if (capri_hbm_read_mem(olist_mem_addr, (uint8_t*)x3, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to read x3 output from memory @ {:x}",
                (uint64_t) olist_mem_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    if (capri_hbm_read_mem(olist_mem_addr + 32, (uint8_t*)y3, 32)) {
        HAL_TRACE_ERR("ECC Point Mul P256: Failed to read y3 output from memory @ {:x}",
                (uint64_t) (olist_mem_addr + 32)); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"x3", (char *)x3, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"y3", (char *)y3, 32);


cleanup:


    if (olist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, olist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for olist content:{:x}",
                    olist_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for ilist content:{:x}",
                    ilist_mem_addr);
        }
    }

    if (olist_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, olist_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for olist DMA Descr: {:x}",
                    olist_dma_descr_addr);
        }
    }

    if (ilist_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, ilist_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for ilist DMA Descr: {:x}",
                    ilist_dma_descr_addr);
        }
    }

    if (ecc_pm_p256_key_idx != -1) {
        ret = pd_crypto_asym_free_key(ecc_pm_p256_key_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free key descriptor");
        }
    }

    if (key_param) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, key_param);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for key param :{:x}",
                    key_param);
        }
    }

    return ret;
}

hal_ret_t capri_barco_asym_ecdsa_p256_sig_gen(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *da,
        uint8_t *k, uint8_t *h, uint8_t *r, uint8_t *s)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint64_t                    ilist_dma_descr_addr = 0, olist_dma_descr_addr = 0;
    uint64_t                    key_dma_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, olist_mem_addr = 0, curr_ptr = 0;
    uint64_t                    key_param_addr = 0;
    barco_asym_descriptor_t     asym_req_descr;
    barco_asym_dma_descriptor_t ilist_dma_descr, olist_dma_descr;
    barco_asym_dma_descriptor_t key_dma_descr;
    int32_t                     ecc_p256_key_idx = -1;
    crypto_asym_key_t           asym_key;
    uint32_t                    req_tag = 0;
    uint32_t                    status = 0;

#undef CAPRI_BARCO_API_NAME
#define CAPRI_BARCO_API_NAME "ECDSA Sig Gen: "

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"p", (char *)p, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"a", (char *)a, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"b", (char *)b, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"n", (char *)n, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"xg", (char *)xg, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"yg", (char *)yg, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"da", (char *)da, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"k", (char *)k, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"h", (char *)h, 32);

    /* Setup params in the key memory */
    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &key_param_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for key param");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for key param @ {:x}", key_param_addr); 

    curr_ptr = key_param_addr;
    
    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)p, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param p into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)n, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param n into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)xg, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param xg into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)yg, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param yg into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)a, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param a into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)b, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param b into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)da, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param da into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &key_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for key DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for ilist DMA Descr @ {:x}", key_dma_descr_addr); 

    /* Setup key DMA descriptor */
    key_dma_descr.address = key_param_addr;
    key_dma_descr.stop = 1;
    key_dma_descr.rsvd0 = 1;
    key_dma_descr.next = 0;
    key_dma_descr.int_en = 0;
    key_dma_descr.discard = 0;
    key_dma_descr.realign = 0;
    key_dma_descr.cst_addr = 0;
    key_dma_descr.length = (curr_ptr - key_param_addr);
    if (capri_hbm_write_mem(key_dma_descr_addr, (uint8_t*)&key_dma_descr,
                sizeof(key_dma_descr))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write key DMA Descr @ {:x}",
                (uint64_t) key_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    ret = pd_crypto_asym_alloc_key(&ecc_p256_key_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate key descriptor");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated Key Descr @ {:x}", ecc_p256_key_idx); 

    asym_key.key_param_list = key_dma_descr_addr;
    asym_key.command_reg = (CAPRI_BARCO_ASYM_CMD_SWAP_BYTES |
                            CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP(32) |
                            CAPRI_BARCO_ASYM_ECDSA_SIG_GEN);
                                
    ret = pd_crypto_asym_write_key(ecc_p256_key_idx, &asym_key);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write key: {}", ecc_p256_key_idx);
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Setup key @ {:x}", ecc_p256_key_idx); 


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &ilist_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for ilist DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for ilist DMA Descr @ {:x}", ilist_dma_descr_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &olist_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for olist DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for olist DMA Descr @ {:x}", olist_dma_descr_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &ilist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for ilist content");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for input mem @ {:x}", ilist_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &olist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for olist content");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for output mem @ {:x}", olist_mem_addr); 

    /* Copy the input to the ilist memory */
    curr_ptr = ilist_mem_addr;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)k, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param k into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)h, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param h into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    /* Setup ilist DMA descriptor */
    ilist_dma_descr.address = ilist_mem_addr;
    ilist_dma_descr.stop = 1;
    ilist_dma_descr.rsvd0 = 1;
    ilist_dma_descr.next = 0;
    ilist_dma_descr.int_en = 0;
    ilist_dma_descr.discard = 0;
    ilist_dma_descr.realign = 0;
    ilist_dma_descr.cst_addr = 0;
    ilist_dma_descr.length = (curr_ptr - ilist_mem_addr);
    if (capri_hbm_write_mem(ilist_dma_descr_addr, (uint8_t*)&ilist_dma_descr,
                sizeof(ilist_dma_descr))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ilist DMA Descr @ {:x}",
                (uint64_t) ilist_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup olist DMA descriptor */
    olist_dma_descr.address = olist_mem_addr;
    olist_dma_descr.stop = 1;
    olist_dma_descr.rsvd0 = 1;
    olist_dma_descr.next = 0;
    olist_dma_descr.int_en = 0;
    olist_dma_descr.discard = 0;
    olist_dma_descr.realign = 0;
    olist_dma_descr.cst_addr = 0;
    olist_dma_descr.length = (2 * 32 ); /* Outputs: r, s */
    if (capri_hbm_write_mem(olist_dma_descr_addr, (uint8_t*)&olist_dma_descr,
                sizeof(olist_dma_descr))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write olist DMA Descr @ {:x}",
                (uint64_t) olist_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Asymmetric Request Descriptor */
    asym_req_descr.input_list_addr = ilist_dma_descr_addr;
    asym_req_descr.output_list_addr = olist_dma_descr_addr;
    asym_req_descr.key_descr_idx_lo = (uint8_t) (ecc_p256_key_idx & 0xff);
    asym_req_descr.key_descr_idx_hi = (uint16_t) (ecc_p256_key_idx >> 8);
    asym_req_descr.status_addr = curr_ptr;
    asym_req_descr.opaque_tag_value = 0;
    asym_req_descr.opage_tag_wr_en = 0;
    asym_req_descr.flag_a = 0;
    asym_req_descr.flag_b = 0;

    ret = capri_barco_ring_queue_request(types::BARCO_RING_ASYM, (void *)&asym_req_descr, &req_tag);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to enqueue request");
        ret = HAL_RET_ERR;
        goto cleanup;
    }
    /* Poll for completion */
    while (capri_barco_ring_poll(types::BARCO_RING_ASYM, req_tag) != TRUE) {
        //HAL_TRACE_DEBUG("ECC Point Mul P256: Waiting for Barco completion...");
    }
    if (capri_hbm_read_mem(asym_req_descr.status_addr, (uint8_t*)&status, sizeof(status))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to retrieve operation status @ {:x}",
                (uint64_t) asym_req_descr.status_addr); 
        ret = HAL_RET_ERR;
        goto cleanup;
    }
    if (status != 0) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Operation failed with status {:x}",
                status); 
        ret = HAL_RET_ERR;
        goto cleanup;
    }

    /* Copy out the results */
    if (capri_hbm_read_mem(olist_mem_addr, (uint8_t*)r, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to read r output from memory @ {:x}",
                (uint64_t) olist_mem_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    if (capri_hbm_read_mem(olist_mem_addr + 32, (uint8_t*)s, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to read s output from memory @ {:x}",
                (uint64_t) (olist_mem_addr + 32)); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"r", (char *)r, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"s", (char *)s, 32);


cleanup:


    if (olist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, olist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for olist content:{:x}",
                    olist_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for ilist content:{:x}",
                    ilist_mem_addr);
        }
    }

    if (olist_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, olist_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for olist DMA Descr: {:x}",
                    olist_dma_descr_addr);
        }
    }

    if (ilist_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, ilist_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for ilist DMA Descr: {:x}",
                    ilist_dma_descr_addr);
        }
    }

    if (ecc_p256_key_idx != -1) {
        ret = pd_crypto_asym_free_key(ecc_p256_key_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME"Failed to free key descriptor");
        }
    }

    if (key_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, key_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for key DMA Descr: {:x}",
                    key_dma_descr_addr);
        }
    }

    if (key_param_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, key_param_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for key param :{:x}",
                    key_param_addr);
        }
    }

    return ret;
}

hal_ret_t capri_barco_asym_ecdsa_p256_sig_verify(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *xq,
        uint8_t *yq, uint8_t *r, uint8_t *s, uint8_t *h)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint64_t                    ilist_dma_descr_addr = 0, olist_dma_descr_addr = 0;
    uint64_t                    key_dma_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, olist_mem_addr = 0, curr_ptr = 0;
    uint64_t                    key_param_addr = 0;
    barco_asym_descriptor_t     asym_req_descr;
    barco_asym_dma_descriptor_t ilist_dma_descr, olist_dma_descr;
    barco_asym_dma_descriptor_t key_dma_descr;
    int32_t                     ecc_p256_key_idx = -1;
    crypto_asym_key_t           asym_key;
    uint32_t                    req_tag = 0;
    uint32_t                    status = 0;

#undef CAPRI_BARCO_API_NAME
#define CAPRI_BARCO_API_NAME "ECDSA Sig Verify: "

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"p", (char *)p, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"a", (char *)a, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"b", (char *)b, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"n", (char *)n, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"xg", (char *)xg, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"yg", (char *)yg, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"xq", (char *)xq, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"yq", (char *)yq, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"r", (char *)r, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"s", (char *)s, 32);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"h", (char *)h, 32);

    /* Setup params in the key memory */
    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &key_param_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for key param");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for key param @ {:x}", key_param_addr); 

    curr_ptr = key_param_addr;
    
    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)p, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param p into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)n, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param n into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)xg, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param xg into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)yg, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param yg into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)a, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param a into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)b, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param b into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)xq, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param xq into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)yq, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param yq into key memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &key_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for key DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for ilist DMA Descr @ {:x}", key_dma_descr_addr); 

    /* Setup key DMA descriptor */
    key_dma_descr.address = key_param_addr;
    key_dma_descr.stop = 1;
    key_dma_descr.rsvd0 = 1;
    key_dma_descr.next = 0;
    key_dma_descr.int_en = 0;
    key_dma_descr.discard = 0;
    key_dma_descr.realign = 0;
    key_dma_descr.cst_addr = 0;
    key_dma_descr.length = (curr_ptr - key_param_addr);
    if (capri_hbm_write_mem(key_dma_descr_addr, (uint8_t*)&key_dma_descr,
                sizeof(key_dma_descr))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write key DMA Descr @ {:x}",
                (uint64_t) key_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    ret = pd_crypto_asym_alloc_key(&ecc_p256_key_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate key descriptor");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated Key Descr @ {:x}", ecc_p256_key_idx); 

    asym_key.key_param_list = key_dma_descr_addr;
    asym_key.command_reg = (CAPRI_BARCO_ASYM_CMD_SWAP_BYTES |
                            CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP(32) |
                            CAPRI_BARCO_ASYM_ECDSA_SIG_VERIFY);
                                
    ret = pd_crypto_asym_write_key(ecc_p256_key_idx, &asym_key);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write key: {}", ecc_p256_key_idx);
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Setup key @ {:x}", ecc_p256_key_idx); 


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &ilist_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for ilist DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for ilist DMA Descr @ {:x}", ilist_dma_descr_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_DMA_DESCR,
            NULL, &olist_dma_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for olist DMA Descr");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for olist DMA Descr @ {:x}", olist_dma_descr_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &ilist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for ilist content");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for input mem @ {:x}", ilist_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
            NULL, &olist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to allocate memory for olist content");
        goto cleanup;
    }
    HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Allocated memory for output mem @ {:x}", olist_mem_addr); 

    /* Copy the input to the ilist memory */
    curr_ptr = ilist_mem_addr;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)r, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param r into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)s, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param s into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)h, 32)) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ECC param h into ilist memory @ {:x}", (uint64_t) curr_ptr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }
    curr_ptr += 32;

    /* Setup ilist DMA descriptor */
    ilist_dma_descr.address = ilist_mem_addr;
    ilist_dma_descr.stop = 1;
    ilist_dma_descr.rsvd0 = 1;
    ilist_dma_descr.next = 0;
    ilist_dma_descr.int_en = 0;
    ilist_dma_descr.discard = 0;
    ilist_dma_descr.realign = 0;
    ilist_dma_descr.cst_addr = 0;
    ilist_dma_descr.length = (curr_ptr - ilist_mem_addr);
    if (capri_hbm_write_mem(ilist_dma_descr_addr, (uint8_t*)&ilist_dma_descr,
                sizeof(ilist_dma_descr))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write ilist DMA Descr @ {:x}",
                (uint64_t) ilist_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup olist DMA descriptor */
    olist_dma_descr.address = olist_mem_addr;
    olist_dma_descr.stop = 1;
    olist_dma_descr.rsvd0 = 1;
    olist_dma_descr.next = 0;
    olist_dma_descr.int_en = 0;
    olist_dma_descr.discard = 0;
    olist_dma_descr.realign = 0;
    olist_dma_descr.cst_addr = 0;
    olist_dma_descr.length = (0 * 32 ); /* No outputs */
    if (capri_hbm_write_mem(olist_dma_descr_addr, (uint8_t*)&olist_dma_descr,
                sizeof(olist_dma_descr))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to write olist DMA Descr @ {:x}",
                (uint64_t) olist_dma_descr_addr); 
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Asymmetric Request Descriptor */
    asym_req_descr.input_list_addr = ilist_dma_descr_addr;
    asym_req_descr.output_list_addr = olist_dma_descr_addr;
    asym_req_descr.key_descr_idx_lo = (uint8_t) (ecc_p256_key_idx & 0xff);
    asym_req_descr.key_descr_idx_hi = (uint16_t) (ecc_p256_key_idx >> 8);
    asym_req_descr.status_addr = curr_ptr;
    asym_req_descr.opaque_tag_value = 0;
    asym_req_descr.opage_tag_wr_en = 0;
    asym_req_descr.flag_a = 0;
    asym_req_descr.flag_b = 0;

    ret = capri_barco_ring_queue_request(types::BARCO_RING_ASYM, (void *)&asym_req_descr, &req_tag);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to enqueue request");
        ret = HAL_RET_ERR;
        goto cleanup;
    }
    /* Poll for completion */
    while (capri_barco_ring_poll(types::BARCO_RING_ASYM, req_tag) != TRUE) {
        //HAL_TRACE_DEBUG("ECC Point Mul P256: Waiting for Barco completion...");
    }
    if (capri_hbm_read_mem(asym_req_descr.status_addr, (uint8_t*)&status, sizeof(status))) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to retrieve operation status @ {:x}",
                (uint64_t) asym_req_descr.status_addr); 
        ret = HAL_RET_ERR;
        goto cleanup;
    }
    if (status != 0) {
        HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Operation failed with status {:x}",
                status); 
        ret = HAL_RET_ERR;
        goto cleanup;
    }
    else {
        HAL_TRACE_DEBUG(CAPRI_BARCO_API_NAME "Signature Verification success");
    }

    /* No output */

cleanup:


    if (olist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, olist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for olist content:{:x}",
                    olist_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("ECC Point Mul P256: Failed to free memory for ilist content:{:x}",
                    ilist_mem_addr);
        }
    }

    if (olist_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, olist_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for olist DMA Descr: {:x}",
                    olist_dma_descr_addr);
        }
    }

    if (ilist_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, ilist_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for ilist DMA Descr: {:x}",
                    ilist_dma_descr_addr);
        }
    }

    if (ecc_p256_key_idx != -1) {
        ret = pd_crypto_asym_free_key(ecc_p256_key_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME"Failed to free key descriptor");
        }
    }

    if (key_dma_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_ASYM_DMA_DESCR, key_dma_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for key DMA Descr: {:x}",
                    key_dma_descr_addr);
        }
    }

    if (key_param_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, key_param_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(CAPRI_BARCO_API_NAME "Failed to free memory for key param :{:x}",
                    key_param_addr);
        }
    }

    if (status)
        ret = HAL_RET_ERR;

    return ret;
}

}    // namespace pd
}    // namespace hal
