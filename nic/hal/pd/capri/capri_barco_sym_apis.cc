#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/pd/capri/capri_barco_res.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
#include "nic/hal/pd/capri/capri_barco_sym_apis.hpp"
#include "nic/hal/pd/capri/capri_barco_asym_apis.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"

namespace hal {
namespace pd {

hal_ret_t capri_barco_sym_hash_process_request (cryptoapis::CryptoApiHashType hash_type, bool generate,
						unsigned char *key, int key_len,
						unsigned char *data, int data_len,
						uint8_t *digest,
						int digest_len)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint64_t                    ilist_msg_descr_addr = 0, olist_msg_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, auth_tag_mem_addr = 0, curr_ptr = 0;
    uint64_t                    status_addr = 0, status = 0;
    barco_symm_req_descriptor_t sym_req_descr;
    barco_sym_msg_descriptor_t  ilist_msg_descr;
    uint32_t                    req_tag = 0;
    int32_t                     hmac_key_descr_idx = -1;


    HAL_TRACE_DEBUG("Running {}-{} test:\n",
                    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input Data bytes:", (char *)data, data_len);

    if (key_len) {
	CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input HMAC Key:", (char *)key, key_len);
    }


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_SYM_MSG_DESCR,
				NULL, &ilist_msg_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for ilist MSG Descr",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for ilist DMA Descr @ {:x}",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    ilist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &ilist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for ilist content",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for input mem @ {:x}",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    ilist_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &auth_tag_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for auth-tag content",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for auth-tag mem @ {:x}",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    auth_tag_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &status_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate memory for storing status",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated memory for status mem @ {:x}",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    status_addr);

    if (key_len) {
        ret = pd_crypto_alloc_key(&hmac_key_descr_idx);
	if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to allocate key descriptor",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYM Hash {}-{}: Allocated HMAC Key Descr @ {:x}",
			CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			hmac_key_descr_idx);

	ret = capri_barco_setup_key(hmac_key_descr_idx, types::CRYPTO_KEY_TYPE_HMAC,
				    (uint8_t *)key, (uint32_t) key_len);
	if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write HMAC Key @ {:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  hmac_key_descr_idx);
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYM Hash {}-{}: Setup HMAC Key @ {:x}",
			CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			hmac_key_descr_idx);
    }

    /* Copy the data input to the ilist memory */
    curr_ptr = ilist_mem_addr;
    
    if (capri_hbm_write_mem(curr_ptr, (uint8_t*)data, data_len)) {
        HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write data bytes into ilist memory @ {:x}",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
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
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
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
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to write input digest @ {:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  (uint64_t) auth_tag_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}

	/*
	 * Also initialize status to 0 at the status-address before we invoke barco.
	 */
        if (capri_hbm_write_mem(status_addr, (uint8_t*)&status,
				sizeof(status))) {
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to initialize status value @ {:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  (uint64_t) status_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    }


    /* Setup Symmetric Request Descriptor */
    memset(&sym_req_descr, 0, sizeof(sym_req_descr));
    sym_req_descr.input_list_addr = ilist_msg_descr_addr;
    sym_req_descr.output_list_addr = olist_msg_descr_addr;

    switch (hash_type) {
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA1:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA1_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA1_Verify_Hash;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA224:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA224_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA224_Verify_Hash;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA256:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA256_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA256_Verify_Hash;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA384:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA384_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA384_Verify_Hash;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA512:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA512_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA512_Verify_Hash;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA1:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA1_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA1_Verify_HMAC;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA224:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA224_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA224_Verify_HMAC;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA256:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA256_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA256_Verify_HMAC;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA384:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA384_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA384_Verify_HMAC;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_SHA512:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA512_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA512_Verify_HMAC;
      break;
    case cryptoapis::CRYPTOAPI_HASHTYPE_HMAC_MD5:
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA512_224:
    case cryptoapis::CRYPTOAPI_HASHTYPE_SHA512_256:
    default:
        HAL_TRACE_ERR("SYM Hash {}-{}: Invalid Hash request",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
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
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        ret = HAL_RET_ERR;
        goto cleanup;
    }

    /* Poll for completion */
    while (capri_barco_ring_poll(types::BARCO_RING_MPP0, req_tag) != TRUE) {
        //HAL_TRACE_DEBUG("SYM Hash {}-{}: Waiting for Barco completion...",
        //                CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
    }
    //sleep(5);
    //abort();

    /* Copy out the results */
    if (generate) {
        if (capri_hbm_read_mem(auth_tag_mem_addr, (uint8_t*)digest, digest_len)) {
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to read output digest at Auth-tag addr @ {:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  (uint64_t) auth_tag_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    } else {
        if (capri_hbm_read_mem(status_addr, (uint8_t*)&status, sizeof(uint64_t))){
	    HAL_TRACE_ERR("SYM Hash {}-{}: Failed to read status at Status addr @ {:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  (uint64_t) status_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYM Hash {}-{}:  Verify - got STATUS {:x}-{} from barco",
			CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			status, barco_symm_err_status_str(status));
    }

cleanup:

    if (status_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, status_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for status addr content:{:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  status_addr);
        }
    }

    if (auth_tag_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, auth_tag_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for auth-tag addr content:{:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  auth_tag_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for ilist content:{:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  ilist_mem_addr);
        }
    }

    if (ilist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, ilist_msg_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYM Hash {}-{}: Failed to free memory for ilist MSG Descr:{:x}",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
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

hal_ret_t capri_barco_sym_aes_encrypt_process_request (capri_barco_symm_enctype_e enc_type, bool encrypt,
						       uint8_t *key, int key_len,
						       uint8_t *header, int header_len,
						       uint8_t *plaintext, int plaintext_len,
						       uint8_t *iv, int iv_len,
						       uint8_t *ciphertext, int ciphertext_len,
						       uint8_t *auth_tag, int auth_tag_len)
{
    hal_ret_t                   ret = HAL_RET_OK;
    uint64_t                    ilist_msg_descr_addr = 0, olist_msg_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, olist_mem_addr = 0, auth_tag_mem_addr = 0;
    uint64_t                    iv_addr = 0, status_addr = 0, status = 0, curr_ptr = 0;
    barco_symm_req_descriptor_t sym_req_descr;
    barco_sym_msg_descriptor_t  ilist_msg_descr, olist_msg_descr;
    uint32_t                    req_tag = 0;
    int32_t                     aes_key_descr_idx = -1;

    HAL_TRACE_DEBUG("Running {}-{} test:\n",
                    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");

    if (encrypt) {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input Data bytes", (char *)plaintext, plaintext_len);
    } else {
        CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input Data bytes", (char *)ciphertext, ciphertext_len);
    }

    if (key_len) {
        CAPRI_BARCO_API_PARAM_HEXDUMP(key_len == 16 ?
				      ((char *)"Input AES-128 Key") : ((char *)"Input AES-256 Key:"),
				      (char *)key, key_len);
    }

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_SYM_MSG_DESCR,
				NULL, &ilist_msg_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for ilist MSG Descr",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for ilist DMA Descr @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    ilist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &ilist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for ilist content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for input mem @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    ilist_mem_addr); 


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_SYM_MSG_DESCR,
				NULL, &olist_msg_descr_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for olist MSG Descr",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for olist DMA Descr @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    olist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &olist_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for olist content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for output mem @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    olist_mem_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &iv_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for IV address content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for IV content @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    iv_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &auth_tag_mem_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for auth-tag content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for auth-tag mem @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    auth_tag_mem_addr); 

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &status_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate memory for storing status",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated memory for status mem @ {:x}",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    status_addr);

    if (key_len) {
        ret = pd_crypto_alloc_key(&aes_key_descr_idx);
	if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to allocate key descriptor",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Allocated AES128 Key Descr @ {:x}",
			capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			aes_key_descr_idx);

	ret = capri_barco_setup_key(aes_key_descr_idx, types::CRYPTO_KEY_TYPE_AES256,
				    (uint8_t *)key, (uint32_t) key_len);
	if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write AES Key @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  aes_key_descr_idx);
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Setup AES Key @ {:x}",
			capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			aes_key_descr_idx);
    }

    /* Copy the header+data input to the ilist memory */
    curr_ptr = ilist_mem_addr;

    if (enc_type == CAPRI_SYMM_ENCTYPE_AES_CCM) {
        uint8_t ccm_header[TLS_AES_CCM_HEADER_SIZE], *ptr;

	memset(ccm_header, 0, sizeof(ccm_header));
	ptr = ccm_header;
        *ptr++ = TLS_AES_CCM_HDR_B0_FLAGS;
        memcpy(ptr, iv, iv_len);
	ptr += iv_len;
        ptr++;
        *(uint16_t *)ptr = encrypt ? htons((uint16_t)plaintext_len) : htons((uint16_t)ciphertext_len);
        ptr += 2;
        *(uint16_t *)ptr = htons(TLS_AES_CCM_AAD_SIZE);
        ptr += 2;
	memcpy(ptr, header, header_len);
	ptr += header_len;
        *ptr = 0;  // 1-byte zero pad

	if (capri_hbm_write_mem(curr_ptr, (uint8_t*)ccm_header, sizeof(ccm_header))) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write header bytes into ilist memory @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
        header_len = TLS_AES_CCM_HEADER_SIZE;
    } else {

        if (capri_hbm_write_mem(curr_ptr, (uint8_t*)header, header_len)) {
	    HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write header bytes into ilist memory @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

    curr_ptr +=  header_len;

    if (encrypt) {
        if (capri_hbm_write_mem(curr_ptr, (uint8_t*)plaintext, plaintext_len)) {
	    HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write plaintext bytes into ilist memory @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    } else {
        if (capri_hbm_write_mem(curr_ptr, (uint8_t*)ciphertext, ciphertext_len)) {
	    HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write ciphertext bytes into ilist memory @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

    /* Copy the IV content */
    if (capri_hbm_write_mem(iv_addr, (uint8_t*)iv, iv_len)) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write IV bytes into ilist memory @ {:x}",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		      (uint64_t) curr_ptr);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Input list SYM MSG descriptor */
    memset(&ilist_msg_descr, 0, sizeof(ilist_msg_descr));
    ilist_msg_descr.A0_addr = ilist_mem_addr;
    ilist_msg_descr.O0_addr_offset = 0;
    ilist_msg_descr.L0_data_length = header_len + plaintext_len;
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
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write ilist MSG Descr @ {:x}",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		      (uint64_t) ilist_msg_descr_addr);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Output list SYM MSG descriptor */
    memset(&olist_msg_descr, 0, sizeof(olist_msg_descr));
    olist_msg_descr.A0_addr = olist_mem_addr;
    olist_msg_descr.O0_addr_offset = 0;
    olist_msg_descr.L0_data_length = header_len + plaintext_len;
    olist_msg_descr.A1_addr = 0;
    olist_msg_descr.O1_addr_offset = 0;
    olist_msg_descr.L1_data_length = 0;
    olist_msg_descr.A2_addr = 0;
    olist_msg_descr.O2_addr_offset = 0;
    olist_msg_descr.L2_data_length = 0;
    olist_msg_descr.next_address = 0;
    olist_msg_descr.reserved = 0;

    if (capri_hbm_write_mem(olist_msg_descr_addr, (uint8_t*)&olist_msg_descr,
			    sizeof(olist_msg_descr))) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write olist MSG Descr @ {:x}",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		      (uint64_t) olist_msg_descr_addr);
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;
    }

    /*
     * If it is a "Decrypt" operation, we want to write the input auth-tag at the
     * auth-tag-addr for barco to read.
     */
    if (!encrypt) {
        if (capri_hbm_write_mem(auth_tag_mem_addr, (uint8_t*)auth_tag,
				auth_tag_len)) {
	    HAL_TRACE_ERR("SYMM Decrypt {}-{}: Failed to write input auth-tag @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) auth_tag_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}

	/*
	 * Also initialize status to 0 at the status-address before we invoke barco.
	 */
        if (capri_hbm_write_mem(status_addr, (uint8_t*)&status,
				sizeof(status))) {
	    HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to write ilist MSG Descr @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) ilist_msg_descr_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

    /* Setup Symmetric Request Descriptor */
    memset(&sym_req_descr, 0, sizeof(sym_req_descr));
    sym_req_descr.input_list_addr = ilist_msg_descr_addr;
    sym_req_descr.output_list_addr = olist_msg_descr_addr;

    switch (enc_type) {
    case CAPRI_SYMM_ENCTYPE_AES_CCM:
      sym_req_descr.command = encrypt ?
	//CAPRI_BARCO_SYM_COMMAND_AES_CCM_Encrypt(iv_len, auth_tag_len) :
	//CAPRI_BARCO_SYM_COMMAND_AES_CCM_Decrypt(iv_len, auth_tag_len);
	CAPRI_BARCO_SYM_COMMAND_AES_CCM_Encrypt(0, auth_tag_len) :
	CAPRI_BARCO_SYM_COMMAND_AES_CCM_Decrypt(0, auth_tag_len);
      break;
    case CAPRI_SYMM_ENCTYPE_AES_SHA256_CBC:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_SHA256_CBC_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_SHA256_CBC_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_SHA384_CBC:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_SHA384_CBC_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_SHA384_CBC_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_CBC_SHA256:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_SHA256_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_SHA256_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_CBC_SHA384:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_SHA384_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_SHA384_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_HMAC_SHA256_CBC:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_HMAC_SHA256_CBC_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_HMAC_SHA256_CBC_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_HMAC_SHA384_CBC:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_HMAC_SHA384_CBC_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_HMAC_SHA384_CBC_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_CBC_HMAC_SHA256:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_HMAC_SHA256_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_HMAC_SHA256_Decrypt;
      break;
    case CAPRI_SYMM_ENCTYPE_AES_CBC_HMAC_SHA384:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_HMAC_SHA384_Encrypt:
        CAPRI_BARCO_SYM_COMMAND_AES_HASH_CBC_HMAC_SHA384_Decrypt;
      break;
    default:
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Invalid Hash request",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        ret = HAL_RET_INVALID_ARG;
        goto cleanup;      
    }

    sym_req_descr.key_descr_idx = aes_key_descr_idx;
    sym_req_descr.iv_address = iv_addr;
    sym_req_descr.auth_tag_addr = auth_tag_mem_addr;
    sym_req_descr.header_size = header_len;
    sym_req_descr.status_addr = status_addr;
    sym_req_descr.opaque_tag_value = 0;
    sym_req_descr.rsvd = 0;
    sym_req_descr.opaque_tag_wr_en = 0;
    sym_req_descr.sector_size = 0;
    sym_req_descr.application_tag = 0;
    sym_req_descr.sector_num = 0;
    sym_req_descr.doorbell_addr = 0; // Currently we use consumer-index to track completions
    sym_req_descr.doorbell_data = 0;

    ciphertext_len = plaintext_len;

    ret = capri_barco_ring_queue_request(types::BARCO_RING_MPP0, (void *)&sym_req_descr,
					 &req_tag);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to enqueue request",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        ret = HAL_RET_ERR;
        goto cleanup;
    }

    /* Poll for completion */
    while (capri_barco_ring_poll(types::BARCO_RING_MPP0, req_tag) != TRUE) {
        //HAL_TRACE_DEBUG("SYMM Encrypt {}-{}: Waiting for Barco completion...",
        //                capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
    }
    //sleep(5);
    //abort();

    /* Copy out the results */
    if (encrypt) {
        if (capri_hbm_read_mem(auth_tag_mem_addr, (uint8_t*)auth_tag, auth_tag_len)) {
	    HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to read output Auth-tag at addr @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) auth_tag_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}

        if (capri_hbm_read_mem(olist_mem_addr + header_len, (uint8_t*)ciphertext, ciphertext_len)) {
	    HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to read output ciphertext at addr @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) olist_mem_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}

    } else {
        if (capri_hbm_read_mem(status_addr, (uint8_t*)&status, sizeof(uint64_t))){
	    HAL_TRACE_ERR("SYMM Decrypt {}-{}: Failed to read status at Status addr @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) status_addr);
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
	HAL_TRACE_DEBUG("SYMM Decrypt {}-{}:  Decrypt - got STATUS {:x}-{} from barco",
			capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			status, barco_symm_err_status_str(status));

        if (capri_hbm_read_mem(olist_mem_addr + header_len, (uint8_t*)plaintext, plaintext_len)) {
	    HAL_TRACE_ERR("SYMM Decrypt {}-{}: Failed to read output decrypted plaintext @ {:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) (olist_mem_addr + header_len));
	    ret = HAL_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

cleanup:

    if (status_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, status_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for status addr content:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  status_addr);
        }
    }

    if (iv_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, iv_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for IV addr content:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  iv_addr);
        }
    }

    if (auth_tag_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, auth_tag_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for auth-tag addr content:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  auth_tag_mem_addr);
        }
    }

    if (olist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, olist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for olist content:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  olist_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for ilist content:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  ilist_mem_addr);
        }
    }

    if (olist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, olist_msg_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for olist MSG Descr:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  olist_msg_descr_addr);
        }
    }

    if (ilist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, ilist_msg_descr_addr);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free memory for ilist MSG Descr:{:x}",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  ilist_msg_descr_addr);
        }
    }

    if (aes_key_descr_idx != -1) {
        ret = pd_crypto_free_key(aes_key_descr_idx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("SYMM Encrypt {}-{}: Failed to free key descriptor @ {:x}",
			  aes_key_descr_idx);
        }
    }

    return encrypt ? ret : (status == 0 ? (hal_ret_t )0 : (hal_ret_t)-1);
}

#define DRBG_SET(X) \
    cap_hens_csr_t& csr_he_ns = CAP_BLK_REG_MODEL_ACCESS(cap_hens_csr_t, chip_id, 0); \
    csr_he_ns.dhs_crypto_ctl.md_drbg_##X.fld(value); \
    if (refresh) { \
        csr_he_ns.dhs_crypto_ctl.md_drbg_##X.set_access_no_zero_time(true); \
        csr_he_ns.dhs_crypto_ctl.md_drbg_##X.write(); \
        csr_he_ns.dhs_crypto_ctl.md_drbg_##X.set_access_no_zero_time(false); }

#define DRBG_GET(X) \
    cap_hens_csr_t& csr_he_ns = CAP_BLK_REG_MODEL_ACCESS(cap_hens_csr_t, chip_id, 0); \
    if (refresh) { \
        csr_he_ns.dhs_crypto_ctl.md_drbg_##X.set_access_no_zero_time(true); \
        csr_he_ns.dhs_crypto_ctl.md_drbg_##X.read(); \
        csr_he_ns.dhs_crypto_ctl.md_drbg_##X.set_access_no_zero_time(false); } \
    return csr_he_ns.dhs_crypto_ctl.md_drbg_##X.fld().convert_to<int>();

static inline
void cap_drbg_set_ctl_isr(int chip_id, int value, bool refresh)
{
    DRBG_SET(isr);
}

static inline
void cap_drbg_set_ctl_msk(int chip_id, int value, bool refresh)
{
    DRBG_SET(msk);
}

static inline
void cap_drbg_set_ctl_icr(int chip_id, int value, bool refresh)
{
    DRBG_SET(icr);
}

static inline
void cap_drbg_set_ctl_ver(int chip_id, int value, bool refresh)
{
    DRBG_SET(ver);
}

static inline
void cap_drbg_set_ctl_gct(int chip_id, int value, bool refresh)
{
    DRBG_SET(gct);
}

static inline
void cap_drbg_set_ctl_gs(int chip_id, int value, bool refresh)
{
    DRBG_SET(gs);
}

static inline
void cap_drbg_set_ctl_rng(int chip_id, int value, bool refresh)
{
    DRBG_SET(rng);
}

static inline
void cap_drbg_set_ctl_drnge(int chip_id, int value, bool refresh)
{
    DRBG_SET(drnge);
}

static inline
int cap_drbg_get_ctl_isr(int chip_id, bool refresh)
{
    DRBG_GET(isr);
}

static inline
int cap_drbg_get_ctl_msk(int chip_id, bool refresh)
{
    DRBG_GET(msk);
}

static inline
int cap_drbg_get_ctl_icr(int chip_id, bool refresh)
{
    DRBG_GET(icr);
}

static inline
int cap_drbg_get_ctl_ver(int chip_id, bool refresh)
{
    DRBG_GET(ver);
}

static inline
int cap_drbg_get_ctl_gct(int chip_id, bool refresh)
{
    DRBG_GET(gct);
}

static inline
int cap_drbg_get_ctl_gs(int chip_id,  bool refresh)
{
    DRBG_GET(gs);
}

static inline
int cap_drbg_get_ctl_rng(int chip_id, bool refresh)
{
    DRBG_GET(rng);
}

static inline
int cap_drbg_get_ctl_drnge(int chip_id, bool refresh)
{
    DRBG_GET(drnge);
}

#define DRBG_WRITE(X) \
    cap_hens_csr_t& csr_he_ns = CAP_BLK_REG_MODEL_ACCESS(cap_hens_csr_t, chip_id, 0); \
    for (int i = 0; i < (len + 3) / 4; i++) { \
        unsigned int t = 0; \
        for (int j = 0; j < 4 && i * 4 + j < len; j++) { \
            t |= (((unsigned int)value[i*4+j]) << ((3-j)*8)); \
        } \
        csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].fld(t); \
        if (refresh) { \
            csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].set_access_no_zero_time(true); \
            csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].write(); \
            csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].set_access_no_zero_time(false); } \
    }

#define DRBG_READ(X) \
    cap_hens_csr_t& csr_he_ns = CAP_BLK_REG_MODEL_ACCESS(cap_hens_csr_t, chip_id, 0); \
    for (int i = 0; i < (len + 3) / 4; i++) { \
        unsigned int t = 0; \
        if (refresh) { \
            csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].set_access_no_zero_time(true); \
            csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].read(); \
            csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].set_access_no_zero_time(false); } \
        t = csr_he_ns.dhs_crypto_ctl.md_drbg_cryptoram_##X[i].fld().convert_to<unsigned int>(); \
        for (int j = 0; j < 4 && i * 4 + j < len; j++) { \
            value[i*4+j] = (unsigned char)(t >> ((3 - j) * 8)); \
        } \
    }

static inline void cap_drbg_write_ram_rand_num0(int chip_id, const unsigned char* value, int len, bool refresh)
{
    DRBG_WRITE(random_num0)
}

static inline void cap_drbg_write_ram_rand_num1(int chip_id, const unsigned char* value, int len, bool refresh)
{
    DRBG_WRITE(random_num1)
}

static inline void cap_drbg_read_ram_rand_num0(int chip_id, unsigned char* value, int len, bool refresh)
{
    DRBG_READ(random_num0)
}

static inline void cap_drbg_read_ram_rand_num1(int chip_id, unsigned char* value, int len, bool refresh)
{
    DRBG_READ(random_num1)
}

void
capri_barco_init_drbg (void)
{
#if 0

    /*
     * Testing only.
     */
    unsigned char num0[512];
    unsigned char num1[512];

    memset(num0, 0, 512);
    memset(num1, 0, 512);
    cap_drbg_read_ram_rand_num0(0, num0, 512, true);
    cap_drbg_read_ram_rand_num1(0, num1, 512, true);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Random number set 0:", (char *)num0, 512);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Random number set 1:", (char *)num1, 512);

    cap_drbg_set_ctl_rng(0, 0x80000000, true);
    cap_drbg_read_ram_rand_num0(0, num0, 512, true);
    cap_drbg_read_ram_rand_num1(0, num1, 512, true);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Random number set 0:", (char *)num0, 512);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Random number set 1:", (char *)num1, 512);
#endif

    /*
     * Generate the random numbers once from ARM cpu, for the first use by the
     * data-path. Data-path program will generate for subsequent uses.
     * (Eventually with a timer-based DRBG producer ring infra in data-path, we'll
     * not need this).
     */
    cap_drbg_set_ctl_rng(0, 0xC0000000, true);

}

}    // namespace pd
}    // namespace hal
