#include "nic/hal/hal.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/pd/capri/capri_barco_res.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
#include "nic/hal/pd/capri/capri_barco_sym_apis.hpp"

namespace hal {
namespace pd {

hal_ret_t capri_barco_sym_hash_process_request (crypto_hash_type_e hash_type, bool generate,
					        char *key, int key_len,
					        char *data, int data_len,
					        uint8_t *digest,
						int digest_len)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint64_t                    ilist_msg_descr_addr = 0, olist_msg_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, auth_tag_mem_addr = 0, curr_ptr = 0;
    uint64_t                    status_addr = 0, status = 0;
    barco_mpp_req_descriptor_t  sym_req_descr;
    barco_sym_msg_descriptor_t  ilist_msg_descr;
    uint32_t                    req_tag = 0;
    int32_t                     hmac_key_descr_idx = -1;

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_SYM_MSG_DESCR,
				NULL, &ilist_msg_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for ilist MSG Descr",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for ilist DMA Descr @ {:x}",
		    crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
		    ilist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &ilist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for ilist content",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for input mem @ {:x}",
		    crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
		    ilist_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &auth_tag_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for auth-tag content",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for auth-tag mem @ {:x}",
		    crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
		    auth_tag_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &status_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for storing status",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for status mem @ {:x}",
		    crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
		    status_addr);

    if (key_len) {
        ret = pd_crypto_alloc_key(&hmac_key_descr_idx);
	if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate key descriptor",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated HMAC Key Descr @ {:x}",
			crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			hmac_key_descr_idx);

	ret = capri_barco_setup_key(hmac_key_descr_idx, types::CRYPTO_KEY_TYPE_HMAC,
				    (uint8_t *)key, (uint32_t) key_len);
	if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write HMAC Key @ {:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  hmac_key_descr_idx);
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYM Hash {}-{}: Setup HMAC Key @ {:x}",
			crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			hmac_key_descr_idx);
    }

    /* Copy the data input to the ilist memory */
    curr_ptr = ilist_mem_addr;
    
    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)data, data_len)) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write data bytes into ilist memory @ {:x}",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
		      (uint64_t) curr_ptr);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Input list SYM MSG descriptor */
    memset(&ilist_msg_descr, 0, sizeof(ilist_msg_descr));
    ilist_msg_descr.A0_addr = ilist_mem_addr;
    ilist_msg_descr.O0_addr_offset = 0;
    ilist_msg_descr.L0_data_length = data_len;
    ilist_msg_descr.A1_addr = 0;
    ilist_msg_descr.O1_addr_offset = 0;
    ilist_msg_descr.L1_data_length = 0;
    ilist_msg_descr.A2_addr = 0;
    ilist_msg_descr.O2_addr_offset = 0;
    ilist_msg_descr.L2_data_length = 0;
    ilist_msg_descr.next_address = 0;
    ilist_msg_descr.reserved = 0;

    if (capri_hbm_write_mem(ilist_msg_descr_addr, (uint8_t*)&ilist_msg_descr,
			    sizeof(ilist_msg_descr))) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write ilist MSG Descr @ {:x}",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
		      (uint64_t) ilist_msg_descr_addr);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /*
     * If it is a "Verify" operation, we want to write the input digest at the
     * auth-tag-addr for barco to read.
     */
    if (!generate) {
        if (capri_hbm_write_mem(auth_tag_mem_addr, (uint8_t*)digest,
				digest_len)) {
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write ilist MSG Descr @ {:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  (uint64_t) ilist_msg_descr_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}

	/*
	 * Also initialize status to 0 at the status-address before we invoke barco.
	 */
        if (capri_hbm_write_mem(status_addr, (uint8_t*)&status,
				sizeof(status))) {
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write ilist MSG Descr @ {:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  (uint64_t) ilist_msg_descr_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    }


    /* Setup Symmetric Request Descriptor */
    memset(&sym_req_descr, 0, sizeof(sym_req_descr));
    sym_req_descr.input_list_addr = ilist_msg_descr_addr;
    sym_req_descr.output_list_addr = olist_msg_descr_addr;

    switch (hash_type) {
    case CRYPTO_HASH_TYPE_SHA1:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA1_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA1_Verify_Hash;
      break;
    case CRYPTO_HASH_TYPE_SHA224:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA224_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA224_Verify_Hash;
      break;
    case CRYPTO_HASH_TYPE_SHA256:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA256_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA256_Verify_Hash;
      break;
    case CRYPTO_HASH_TYPE_SHA384:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA384_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA384_Verify_Hash;
      break;
    case CRYPTO_HASH_TYPE_SHA512:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA512_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA512_Verify_Hash;
      break;
    case CRYPTO_HASH_TYPE_HMAC_SHA1:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA1_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA1_Verify_HMAC;
      break;
    case CRYPTO_HASH_TYPE_HMAC_SHA224:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA224_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA224_Verify_HMAC;
      break;
    case CRYPTO_HASH_TYPE_HMAC_SHA256:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA256_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA256_Verify_HMAC;
      break;
    case CRYPTO_HASH_TYPE_HMAC_SHA384:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA384_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA384_Verify_HMAC;
      break;
    case CRYPTO_HASH_TYPE_HMAC_SHA512:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA512_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA512_Verify_HMAC;
      break;
    case CRYPTO_HASH_TYPE_HMAC_MD5:
    case CRYPTO_HASH_TYPE_SHA512_224:
    case CRYPTO_HASH_TYPE_SHA512_256:
    default:
        HAL_TRACE_ERR("SYM Hash {}-{}: Invalid Hash request",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;      
    }

    sym_req_descr.key_descr_idx = hmac_key_descr_idx;
    sym_req_descr.iv_address = 0;
    sym_req_descr.auth_tag_addr = auth_tag_mem_addr;
    sym_req_descr.header_size = 0;
    sym_req_descr.status_addr = status_addr;
    sym_req_descr.opaque_tag_value = 0;
    sym_req_descr.rsvd = 0;
    sym_req_descr.opaque_tag_wr_en = 0;
    sym_req_descr.sector_size = 0;
    sym_req_descr.application_tag = 0;
    sym_req_descr.sector_num = 0;
    sym_req_descr.doorbell_addr = 0; // Currently we use consumer-index to track completions
    sym_req_descr.doorbell_data = 0;

    ret = capri_barco_ring_queue_request(types::BARCO_RING_MPP0, (void *)&sym_req_descr,
					 &req_tag);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to enqueue request",
		      crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
        ret = HAL_RET_ERR;
        goto cleanup;
    }

    /* Poll for completion */
    while (capri_barco_ring_poll(types::BARCO_RING_MPP0, req_tag) != TRUE) {
        //HAL_TRACE_DEBUG("SYM Hash {}-{}: Waiting for Barco completion...",
        //                crypto_hash_type_str(hash_type), generate ? "generate" : "verify");
    }
    //sleep(5);
    //abort();

    /* Copy out the results */
    if (generate) {
        if (capri_hbm_read_mem(auth_tag_mem_addr, (uint8_t*)digest, digest_len)) {
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to read output digest at Auth-tag addr @ {:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  (uint64_t) auth_tag_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    } else {
        if (capri_hbm_read_mem(status_addr, (uint8_t*)&status, sizeof(uint64_t))){
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to read output digest at Auth-tag addr @ {:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  (uint64_t) auth_tag_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYM Hash {}-{}:  Verify - got STATUS {:x}-{} from barco",
			crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			status, barco_symm_err_status_str(status));
    }

cleanup:

    if (status_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, status_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for status addr content:{:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  status_addr);
        }
    }

    if (auth_tag_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, auth_tag_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for auth-tag addr content:{:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  auth_tag_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for ilist content:{:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  ilist_mem_addr);
        }
    }

    if (ilist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, ilist_msg_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for ilist MSG Descr:{:x}",
			  crypto_hash_type_str(hash_type), generate ? "generate" : "verify",
			  ilist_msg_descr_addr);
        }
    }

    if (hmac_key_descr_idx != -1) {
        ret = pd_crypto_free_key(hmac_key_descr_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free key descriptor @ {:x}",
			  hmac_key_descr_idx);
        }
    }

    return generate ? ret : (status == 0 ? (hal_ret_t )0 : (hal_ret_t)-1);
}

}    // namespace pd
}    // namespace hal
