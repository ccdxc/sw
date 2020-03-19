#include <arpa/inet.h>
#include "include/sdk/crypto_apis.hpp"
#include "platform/capri/capri_barco_crypto.hpp"
#include "platform/capri/capri_barco_res.hpp"
#include "platform/capri/capri_barco_rings.hpp"
#include "platform/capri/capri_barco_sym_apis.hpp"
#include "platform/capri/capri_barco_asym_apis.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "platform/capri/capri_common.hpp"
#include "platform/capri/capri_barco.h"
#include "asic/rw/asicrw.hpp"
#include "platform/capri/capri_hbm_rw.hpp"

namespace sdk {
namespace platform {
namespace capri {

static inline sdk_ret_t sha3_validate_digest_len(unsigned short digest_len)
{
    if ((digest_len != 28) &&
            (digest_len != 32) &&
            (digest_len != 48) &&
            (digest_len != 64)) {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t sha3_pad_gen(int data_len, int digest_len, char *sha3_pad, int *sha3_pad_len)
{
    int         r = 0, q = 0;
    char        *pad_offset = sha3_pad;
    sdk_ret_t   ret = SDK_RET_OK;

    ret = sha3_validate_digest_len(digest_len);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /* compute r */
    r = 1600 - (2 * (digest_len * 8));
    /* compute q */
    q = (r/8) - data_len % (r/8);

    if (q == 1) {
        *pad_offset = 0x86;
    }
    else if (q == 2) {
        *pad_offset = 0x06;
        pad_offset++;
        *pad_offset = 0x80;
    }
    else {
        *pad_offset = 0x06;
        pad_offset++;
        bzero(pad_offset, q-2);
        pad_offset += (q-2);
        *pad_offset = 0x80;
    }
    *sha3_pad_len = q;
    return SDK_RET_OK;
}

sdk_ret_t capri_barco_sym_hash_process_request (CryptoApiHashType hash_type, bool generate,
						unsigned char *key, int key_len,
						unsigned char *data, int data_len,
						uint8_t *digest,
						int digest_len)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    uint64_t                    ilist_msg_descr_addr = 0, olist_msg_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, auth_tag_mem_addr = 0, curr_ptr = 0;
    uint64_t                    status_addr = 0, status = 0;
    barco_symm_req_descriptor_t sym_req_descr;
    barco_sym_msg_descriptor_t  ilist_msg_descr;
    uint32_t                    req_tag = 0;
    int32_t                     hmac_key_descr_idx = -1;
#define     SHA3_PAD_MAX_SZ 144
    char                        sha3_pad[SHA3_PAD_MAX_SZ];
    int                         sha3_pad_len = 0;


    SDK_TRACE_DEBUG("Running %s-%s test:\n",
                    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");

    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input Data bytes:", (char *)data, data_len);

    if (key_len) {
	CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Input HMAC Key:", (char *)key, key_len);
    }


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_SYM_MSG_DESCR,
				NULL, &ilist_msg_descr_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to allocate memory for ilist MSG Descr",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYM Hash %s-%s: Allocated memory for ilist DMA Descr @ 0x%llx",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    ilist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512KB,
				NULL, &ilist_mem_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to allocate memory for ilist content",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYM Hash %s-%s: Allocated memory for input mem @ 0x%llx",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    ilist_mem_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &auth_tag_mem_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to allocate memory for auth-tag content",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYM Hash %s-%s: Allocated memory for auth-tag mem @ 0x%llx",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    auth_tag_mem_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &status_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to allocate memory for storing status",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYM Hash %s-%s: Allocated memory for status mem @ 0x%llx",
		    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		    status_addr);

    if (key_len) {
        ret = capri_barco_sym_alloc_key(&hmac_key_descr_idx);
	if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to allocate key descriptor",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
	    goto cleanup;
	}
	SDK_TRACE_DEBUG("SYM Hash %s-%s: Allocated HMAC Key Descr @ %d",
			CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			hmac_key_descr_idx);

	ret = capri_barco_setup_key(hmac_key_descr_idx, CRYPTO_KEY_TYPE_HMAC,
				    (uint8_t *)key, (uint32_t) key_len);
	if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to write HMAC Key @ %d",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  hmac_key_descr_idx);
	    goto cleanup;
	}
	SDK_TRACE_DEBUG("SYM Hash %s-%s: Setup HMAC Key @ %d",
			CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			hmac_key_descr_idx);
    }

    /* Copy the data input to the ilist memory */
    curr_ptr = ilist_mem_addr;
    if (sdk::asic::asic_mem_write(curr_ptr, (uint8_t*)data, data_len)) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to write data bytes into ilist memory @ 0x%llx",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		      (uint64_t) curr_ptr);
        ret = SDK_RET_INVALID_ARG;
        goto cleanup;
    }

    /* SHA3 needs software padding since the H/W does not support it */
    switch (hash_type) {
        case CRYPTOAPI_HASHTYPE_SHA3_224:
        case CRYPTOAPI_HASHTYPE_SHA3_256:
        case CRYPTOAPI_HASHTYPE_SHA3_384:
        case CRYPTOAPI_HASHTYPE_SHA3_512:
            ret = sha3_pad_gen(data_len, digest_len, sha3_pad, &sha3_pad_len);
            if (ret != SDK_RET_OK) {
                SDK_TRACE_ERR("SYM Hash %s-%s: Failed to generate pad",
                        CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
                goto cleanup;
            }
            //CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"SHA3 Pad:", (char *)sha3_pad, sha3_pad_len);

            curr_ptr = ilist_mem_addr + data_len;
            data_len += sha3_pad_len;

            if (data_len > (512* 1024)) {
                SDK_TRACE_ERR("SYM Hash %s-%s: Total input length (%d - including SHA3 pad) exceeds buffer size %d",
                        CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify", data_len, (512 * 1024));
                goto cleanup;
            }

            if (sdk::asic::asic_mem_write(curr_ptr, (uint8_t*)sha3_pad, sha3_pad_len)) {
                SDK_TRACE_ERR("SYM Hash %s-%s: Failed to write SHA3 pad bytes into ilist memory @ 0x%llx",
                        CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
                        (uint64_t) curr_ptr);
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            break;
        default:
            /* Do nothing */
            break;
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

    if (sdk::asic::asic_mem_write(ilist_msg_descr_addr, (uint8_t*)&ilist_msg_descr,
			    sizeof(ilist_msg_descr))) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to write ilist MSG Descr @ 0x%llx",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
		      (uint64_t) ilist_msg_descr_addr);
        ret = SDK_RET_INVALID_ARG;
        goto cleanup;
    }

    /*
     * If it is a "Verify" operation, we want to write the input digest at the
     * auth-tag-addr for barco to read.
     */
    if (!generate) {
        if (sdk::asic::asic_mem_write(auth_tag_mem_addr, (uint8_t*)digest,
				digest_len)) {
	    SDK_TRACE_ERR("SYM Hash %s-%s: Failed to write input digest @ 0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  (uint64_t) auth_tag_mem_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}

	/*
	 * Also initialize status to 0 at the status-address before we invoke barco.
	 */
        if (sdk::asic::asic_mem_write(status_addr, (uint8_t*)&status,
				sizeof(status))) {
	    SDK_TRACE_ERR("SYM Hash %s-%s: Failed to initialize status value @ 0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  (uint64_t) status_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
    }


    /* Setup Symmetric Request Descriptor */
    memset(&sym_req_descr, 0, sizeof(sym_req_descr));
    sym_req_descr.input_list_addr = ilist_msg_descr_addr;
    sym_req_descr.output_list_addr = olist_msg_descr_addr;

    switch (hash_type) {
    case CRYPTOAPI_HASHTYPE_SHA1:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA1_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA1_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA224:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA224_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA224_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA256:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA256_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA256_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA384:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA384_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA384_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA512:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA512_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA512_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA1:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA1_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA1_Verify_HMAC;
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA224:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA224_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA224_Verify_HMAC;
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA256:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA256_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA256_Verify_HMAC;
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA384:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA384_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA384_Verify_HMAC;
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_SHA512:
      sym_req_descr.command = generate ?
	CAPRI_BARCO_SYM_COMMAND_SHA512_Generate_HMAC : CAPRI_BARCO_SYM_COMMAND_SHA512_Verify_HMAC;
      break;
    case CRYPTOAPI_HASHTYPE_SHA3_224:
      sym_req_descr.command = generate ?
    CAPRI_BARCO_SYM_COMMAND_SHA3_224_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA3_224_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA3_256:
      sym_req_descr.command = generate ?
    CAPRI_BARCO_SYM_COMMAND_SHA3_256_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA3_256_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA3_384:
      sym_req_descr.command = generate ?
    CAPRI_BARCO_SYM_COMMAND_SHA3_384_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA3_384_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_SHA3_512:
      sym_req_descr.command = generate ?
    CAPRI_BARCO_SYM_COMMAND_SHA3_512_Generate_Hash : CAPRI_BARCO_SYM_COMMAND_SHA3_512_Verify_Hash;
      break;
    case CRYPTOAPI_HASHTYPE_HMAC_MD5:
    case CRYPTOAPI_HASHTYPE_SHA512_224:
    case CRYPTOAPI_HASHTYPE_SHA512_256:
    default:
        SDK_TRACE_ERR("SYM Hash %s-%s: Invalid Hash request",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        ret = SDK_RET_INVALID_ARG;
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

    ret = capri_barco_ring_queue_request(BARCO_RING_MPP0, (void *)&sym_req_descr,
					 &req_tag, true);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYM Hash %s-%s: Failed to enqueue request",
		      CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
        ret = SDK_RET_ERR;
        goto cleanup;
    }

    /* Poll for completion */
    while (capri_barco_ring_poll(BARCO_RING_MPP0, req_tag) != TRUE) {
        //SDK_TRACE_DEBUG("SYM Hash %s-%s: Waiting for Barco completion...",
        //                CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify");
    }

    /* Copy out the results */
    if (generate) {
        if (sdk::asic::asic_mem_read(auth_tag_mem_addr, (uint8_t*)digest, digest_len)) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to read output digest at Auth-tag addr @ 0x%llx",
                    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
                    (uint64_t) auth_tag_mem_addr);
            ret = SDK_RET_INVALID_ARG;
            goto cleanup;
        }
        //CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Digest:", (char *)digest, digest_len);
    } else {
        if (sdk::asic::asic_mem_read(status_addr, (uint8_t*)&status, sizeof(uint64_t))){
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to read status at Status addr @ 0x%llx",
                    CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
                    (uint64_t) status_addr);
            ret = SDK_RET_INVALID_ARG;
            goto cleanup;
        }
        SDK_TRACE_DEBUG("SYM Hash %s-%s:  Verify - got STATUS 0x%llx-%s from barco",
                CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
                status, barco_symm_err_status_str(status));
    }

cleanup:

    if (status_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, status_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to free memory for status addr content:0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  status_addr);
        }
    }

    if (auth_tag_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, auth_tag_mem_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to free memory for auth-tag addr content:0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  auth_tag_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512KB, ilist_mem_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to free memory for ilist content:0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  ilist_mem_addr);
        }
    }

    if (ilist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, ilist_msg_descr_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to free memory for ilist MSG Descr:0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  ilist_msg_descr_addr);
        }
    }

    if (hmac_key_descr_idx != -1) {
        ret = capri_barco_sym_free_key(hmac_key_descr_idx);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYM Hash %s-%s: Failed to free key descriptor @ 0x%llx",
			  CryptoApiHashType_Name(hash_type), generate ? "generate" : "verify",
			  hmac_key_descr_idx);
        }
    }

    return generate ? ret : (status == 0 ? (sdk_ret_t )0 : (sdk_ret_t)-1);
}

sdk_ret_t capri_barco_sym_aes_encrypt_process_request (capri_barco_symm_enctype_e enc_type, bool encrypt,
						       uint8_t *key, int key_len,
						       uint8_t *header, int header_len,
						       uint8_t *plaintext, int plaintext_len,
						       uint8_t *iv, int iv_len,
						       uint8_t *ciphertext, int ciphertext_len,
						       uint8_t *auth_tag, int auth_tag_len,
						       bool schedule_barco)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    uint64_t                    ilist_msg_descr_addr = 0, olist_msg_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, olist_mem_addr = 0, auth_tag_mem_addr = 0;
    uint64_t                    iv_addr = 0, status_addr = 0, status = 0, curr_ptr = 0;
    barco_symm_req_descriptor_t sym_req_descr;
    barco_sym_msg_descriptor_t  ilist_msg_descr, olist_msg_descr;
    uint32_t                    req_tag = 0;
    int32_t                     aes_key_descr_idx = -1;

    SDK_TRACE_DEBUG("Running %s-%s test:\n",
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
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for ilist MSG Descr",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for ilist DMA Descr @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    ilist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &ilist_mem_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for ilist content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for input mem @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    ilist_mem_addr);


    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_SYM_MSG_DESCR,
				NULL, &olist_msg_descr_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for olist MSG Descr",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for olist DMA Descr @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    olist_msg_descr_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &olist_mem_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for olist content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for output mem @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    olist_mem_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &iv_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for IV address content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for IV content @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    iv_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &auth_tag_mem_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for auth-tag content",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for auth-tag mem @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    auth_tag_mem_addr);

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &status_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate memory for storing status",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        goto cleanup;
    }
    SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated memory for status mem @ 0x%llx",
		    capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		    status_addr);

    if (key_len) {
        ret = capri_barco_sym_alloc_key(&aes_key_descr_idx);
        // ret = pd_crypto_alloc_key(&aes_key_descr_idx);
	if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to allocate key descriptor",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
	    goto cleanup;
	}
	SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Allocated AES128 Key Descr @ 0x%llx",
			capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			aes_key_descr_idx);

	ret = capri_barco_setup_key(aes_key_descr_idx, CRYPTO_KEY_TYPE_AES256,
				    (uint8_t *)key, (uint32_t) key_len);
	if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write AES Key @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  aes_key_descr_idx);
	    goto cleanup;
	}
	SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Setup AES Key @ 0x%llx",
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

	if (sdk::asic::asic_mem_write(curr_ptr, (uint8_t*)ccm_header, sizeof(ccm_header))) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write header bytes into ilist memory @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
        header_len = TLS_AES_CCM_HEADER_SIZE;
    } else {

        if (sdk::asic::asic_mem_write(curr_ptr, (uint8_t*)header, header_len)) {
	    SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write header bytes into ilist memory @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

    curr_ptr +=  header_len;

    if (encrypt) {
        if (sdk::asic::asic_mem_write(curr_ptr, (uint8_t*)plaintext, plaintext_len)) {
	    SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write plaintext bytes into ilist memory @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
    } else {
        if (sdk::asic::asic_mem_write(curr_ptr, (uint8_t*)ciphertext, ciphertext_len)) {
	    SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write ciphertext bytes into ilist memory @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) curr_ptr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

    /* Copy the IV content */
    if (sdk::asic::asic_mem_write(iv_addr, (uint8_t*)iv, iv_len)) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write IV bytes into ilist memory @ 0x%llx",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		      (uint64_t) curr_ptr);
        ret = SDK_RET_INVALID_ARG;
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

    if (sdk::asic::asic_mem_write(ilist_msg_descr_addr, (uint8_t*)&ilist_msg_descr,
			    sizeof(ilist_msg_descr))) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write ilist MSG Descr @ 0x%llx",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		      (uint64_t) ilist_msg_descr_addr);
        ret = SDK_RET_INVALID_ARG;
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

    if (sdk::asic::asic_mem_write(olist_msg_descr_addr, (uint8_t*)&olist_msg_descr,
			    sizeof(olist_msg_descr))) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write olist MSG Descr @ 0x%llx",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		      (uint64_t) olist_msg_descr_addr);
        ret = SDK_RET_INVALID_ARG;
        goto cleanup;
    }

    /*
     * If it is a "Decrypt" operation, we want to write the input auth-tag at the
     * auth-tag-addr for barco to read.
     */
    if (!encrypt) {
        if (sdk::asic::asic_mem_write(auth_tag_mem_addr, (uint8_t*)auth_tag,
				auth_tag_len)) {
	    SDK_TRACE_ERR("SYMM Decrypt %s-%s: Failed to write input auth-tag @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) auth_tag_mem_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}

	/*
	 * Also initialize status to 0 at the status-address before we invoke barco.
	 */
        if (sdk::asic::asic_mem_write(status_addr, (uint8_t*)&status,
				sizeof(status))) {
	    SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to write ilist MSG Descr @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) ilist_msg_descr_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

    /* Setup Symmetric Request Descriptor */
    memset(&sym_req_descr, 0, sizeof(sym_req_descr));
    sym_req_descr.input_list_addr = ilist_msg_descr_addr;
    sym_req_descr.output_list_addr = olist_msg_descr_addr;

    switch (enc_type) {
    case CAPRI_SYMM_ENCTYPE_AES_GCM:
      sym_req_descr.command = encrypt ?
	CAPRI_BARCO_SYM_COMMAND_AES_GCM_Encrypt :
	CAPRI_BARCO_SYM_COMMAND_AES_GCM_Decrypt;
      break;
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
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Invalid Hash request",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        ret = SDK_RET_INVALID_ARG;
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

    if (enc_type == CAPRI_SYMM_ENCTYPE_AES_GCM) {
        ret = capri_barco_ring_queue_request(BARCO_RING_GCM0, (void *)&sym_req_descr,
					     &req_tag, schedule_barco);
    } else {
        ret = capri_barco_ring_queue_request(BARCO_RING_MPP0, (void *)&sym_req_descr,
					     &req_tag, schedule_barco);
    }

    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to enqueue request",
		      capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        ret = SDK_RET_ERR;
        goto cleanup;
    }

    if (!schedule_barco) {
        SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Request at slot %d, Barco PI not updated",
		        capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
		        req_tag);
        ret = SDK_RET_OK;
	return(ret);
    }

    /* Poll for completion */
    if (enc_type == CAPRI_SYMM_ENCTYPE_AES_GCM) {
        while (capri_barco_ring_poll(BARCO_RING_GCM0, req_tag) != TRUE) {
           //SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Waiting for Barco completion...",
           //                capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        }
    } else {
        while (capri_barco_ring_poll(BARCO_RING_MPP0, req_tag) != TRUE) {
            //SDK_TRACE_DEBUG("SYMM Encrypt %s-%s: Waiting for Barco completion...",
            //                capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt");
        }
    }

    /* Copy out the results */
    if (encrypt) {
        if (sdk::asic::asic_mem_read(auth_tag_mem_addr, (uint8_t*)auth_tag, auth_tag_len)) {
	    SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to read output Auth-tag at addr @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) auth_tag_mem_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}

        if (sdk::asic::asic_mem_read(olist_mem_addr + header_len, (uint8_t*)ciphertext, ciphertext_len)) {
	    SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to read output ciphertext at addr @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) olist_mem_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}

    } else {
        if (sdk::asic::asic_mem_read(status_addr, (uint8_t*)&status, sizeof(uint64_t))){
	    SDK_TRACE_ERR("SYMM Decrypt %s-%s: Failed to read status at Status addr @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) status_addr);
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
	SDK_TRACE_DEBUG("SYMM Decrypt %s-%s:  Decrypt - got STATUS 0x%llx-%s from barco",
			capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			status, barco_symm_err_status_str(status));

        if (sdk::asic::asic_mem_read(olist_mem_addr + header_len, (uint8_t*)plaintext, plaintext_len)) {
	    SDK_TRACE_ERR("SYMM Decrypt %s-%s: Failed to read output decrypted plaintext @ 0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  (uint64_t) (olist_mem_addr + header_len));
	    ret = SDK_RET_INVALID_ARG;
	    goto cleanup;
	}
    }

cleanup:

    if (status_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, status_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for status addr content:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  status_addr);
        }
    }

    if (iv_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, iv_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for IV addr content:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  iv_addr);
        }
    }

    if (auth_tag_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, auth_tag_mem_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for auth-tag addr content:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  auth_tag_mem_addr);
        }
    }

    if (olist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, olist_mem_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for olist content:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  olist_mem_addr);
        }
    }

    if (ilist_mem_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_HBM_MEM_512B, ilist_mem_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for ilist content:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  ilist_mem_addr);
        }
    }

    if (olist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, olist_msg_descr_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for olist MSG Descr:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  olist_msg_descr_addr);
        }
    }

    if (ilist_msg_descr_addr) {
        ret = capri_barco_res_free(CRYPTO_BARCO_RES_SYM_MSG_DESCR, ilist_msg_descr_addr);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free memory for ilist MSG Descr:0x%llx",
			  capri_barco_symm_enctype_name(enc_type), encrypt ? "encrypt" : "decrypt",
			  ilist_msg_descr_addr);
        }
    }

    if (aes_key_descr_idx != -1) {
        ret = capri_barco_sym_free_key(aes_key_descr_idx);
        // ret = pd_crypto_free_key(aes_key_descr_idx);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("SYMM Encrypt %s-%s: Failed to free key descriptor @ %d",
                          capri_barco_symm_enctype_name(enc_type),
                          encrypt ? "encrypt" : "decrypt",
                          aes_key_descr_idx);
        }
    }

    return encrypt ? ret : (status == 0 ? (sdk_ret_t )0 : (sdk_ret_t)-1);
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

static inline void cap_drbg_write_ram_psnl_str_p(int chip_id, const unsigned char* value, int len, bool refresh)
{
    DRBG_WRITE(psnl_str_p)
}

#define TRNG_SET(X) \
    cap_hens_csr_t& csr_he_ns = CAP_BLK_REG_MODEL_ACCESS(cap_hens_csr_t, chip_id, 0); \
    csr_he_ns.dhs_crypto_ctl.md_trng_##X.fld(value); \
    if (refresh) { \
        csr_he_ns.dhs_crypto_ctl.md_trng_##X.set_access_no_zero_time(true); \
        csr_he_ns.dhs_crypto_ctl.md_trng_##X.write(); \
        csr_he_ns.dhs_crypto_ctl.md_trng_##X.set_access_no_zero_time(false); }

#define TRNG_GET(X) \
    cap_hens_csr_t& csr_he_ns = CAP_BLK_REG_MODEL_ACCESS(cap_hens_csr_t, chip_id, 0); \
    if (refresh) { \
        csr_he_ns.dhs_crypto_ctl.md_trng_##X.set_access_no_zero_time(true); \
        csr_he_ns.dhs_crypto_ctl.md_trng_##X.read(); \
        csr_he_ns.dhs_crypto_ctl.md_trng_##X.set_access_no_zero_time(false); } \
    return csr_he_ns.dhs_crypto_ctl.md_trng_##X.fld().convert_to<int>();


static inline void cap_trng_set_ctl_reg(int chip_id, int value, bool refresh)
{
    TRNG_SET(controlreg);
}

static inline void cap_trng_set_fifo_level(int chip_id, int value, bool refresh)
{
    TRNG_SET(fifolevel);
}

static inline void cap_trng_set_fifo_thresh(int chip_id, int value, bool refresh)
{
    TRNG_SET(fifothresh);
}

static inline void cap_trng_set_fifo_depth(int chip_id, int value, bool refresh)
{
    TRNG_SET(fifodepth);
}


static inline void cap_trng_set_key0(int chip_id, int value, bool refresh)
{
    TRNG_SET(key0);
}

static inline void cap_trng_set_key1(int chip_id, int value, bool refresh)
{
    TRNG_SET(key1);
}

static inline void cap_trng_set_key2(int chip_id, int value, bool refresh)
{
    TRNG_SET(key2);
}

static inline void cap_trng_set_key3(int chip_id, int value, bool refresh)
{
    TRNG_SET(key3);
}

static inline
void cap_trng_set_test_data(int chip_id, int value, bool refresh)
{
    TRNG_SET(testdata);
}

static inline void cap_trng_set_rep_thresh(int chip_id, int value, bool refresh)
{
    TRNG_SET(repthresh);
}

static inline void cap_trng_set_prop1(int chip_id, int value, bool refresh)
{
    TRNG_SET(prop1);
}

static inline void cap_trng_set_prop2(int chip_id, int value, bool refresh)
{
    TRNG_SET(prop2);
}

static inline void cap_trng_set_status(int chip_id, int value, bool refresh)
{
    TRNG_SET(status);
}

static inline void cap_trng_set_init_wait_val(int chip_id, int value, bool refresh)
{
    TRNG_SET(initwaitval);
}

static inline void cap_trng_set_disable_osc0(int chip_id, int value, bool refresh)
{
    TRNG_SET(disableosc0);
}

static inline void cap_trng_set_disable_osc1(int chip_id, int value, bool refresh)
{
    TRNG_SET(disableosc1);
}

static inline void cap_trng_set_swoff_tmrval(int chip_id, int value, bool refresh)
{
    TRNG_SET(swofftmrval);
}

static inline void cap_trng_set_clk_div(int chip_id, int value, bool refresh)
{
    TRNG_SET(clkdiv);
}

static inline void cap_trng_set_ais31_conf0(int chip_id, int value, bool refresh)
{
    TRNG_SET(ais31conf0);
}

static inline void cap_trng_set_ais31_conf1(int chip_id, int value, bool refresh)
{
    TRNG_SET(ais31conf1);
}

static inline void cap_trng_set_ais31_conf2(int chip_id, int value, bool refresh)
{
    TRNG_SET(ais31conf2);
}

static inline void cap_trng_set_ais31_status(int chip_id, int value, bool refresh)
{
    TRNG_SET(ais31status);
}

static inline int cap_trng_get_ctl_reg(int chip_id, bool refresh)
{
    TRNG_GET(controlreg);
}

static inline int cap_trng_get_fifo_level(int chip_id, bool refresh)
{
    TRNG_GET(fifolevel);
}

static inline int cap_trng_get_fifo_thresh(int chip_id, bool refresh)
{
    TRNG_GET(fifothresh);
}

static inline int cap_trng_get_fifo_depth(int chip_id, bool refresh)
{
    TRNG_GET(fifodepth);
}


static inline int cap_trng_get_key0(int chip_id, bool refresh)
{
    TRNG_GET(key0);
}

static inline int cap_trng_get_key1(int chip_id, bool refresh)
{
    TRNG_GET(key1);
}

static inline int cap_trng_get_key2(int chip_id, bool refresh)
{
    TRNG_GET(key2);
}

static inline int cap_trng_get_key3(int chip_id, bool refresh)
{
    TRNG_GET(key3);
}

static inline int cap_trng_get_test_data(int chip_id, bool refresh)
{
    TRNG_GET(testdata);
}

static inline int cap_trng_get_rep_thresh(int chip_id, bool refresh)
{
    TRNG_GET(repthresh);
}

static inline int cap_trng_get_prop1(int chip_id, bool refresh)
{
    TRNG_GET(prop1);
}

static inline int cap_trng_get_prop2(int chip_id, bool refresh)
{
    TRNG_GET(prop2);
}

static inline int cap_trng_get_status(int chip_id, bool refresh)
{
    TRNG_GET(status);
}

static inline int cap_trng_get_init_wait_val(int chip_id, bool refresh)
{
    TRNG_GET(initwaitval);
}

static inline int cap_trng_get_disable_osc0(int chip_id, bool refresh)
{
    TRNG_GET(disableosc0);
}

static inline int cap_trng_get_disable_osc1(int chip_id, bool refresh)
{
    TRNG_GET(disableosc1);
}

static inline int cap_trng_get_swoff_tmrval(int chip_id, bool refresh)
{
    TRNG_GET(swofftmrval);
}

static inline int cap_trng_get_clk_div(int chip_id, bool refresh)
{
    TRNG_GET(clkdiv);
}

static inline int cap_trng_get_ais31_conf0(int chip_id, bool refresh)
{
    TRNG_GET(ais31conf0);
}

static inline int cap_trng_get_ais31_conf1(int chip_id, bool refresh)
{
    TRNG_GET(ais31conf1);
}

static inline int cap_trng_get_ais31_conf2(int chip_id, bool refresh)
{
    TRNG_GET(ais31conf2);
}

static inline int cap_trng_get_ais31_status(int chip_id, bool refresh)
{
    TRNG_GET(ais31status);
}

void
capri_barco_init_drbg (void)
{
    unsigned char num0[512];
    unsigned char psnl_str_p[32];
    uint32_t      psnl_str_p_len;
    uint32_t      rng, gs, val;

    /*
     * Initialize the Barco TRNG module.
     */
    SDK_TRACE_DEBUG("[DRBG] initializing ...");
    strcpy((char *)psnl_str_p, "cap/he/drbg/pensando-pers-string"); // Less than 32 bytes
    psnl_str_p_len = strlen((const char *)psnl_str_p);
    SDK_TRACE_DEBUG("[DRBG] generated personalized string P -- %s len %d", psnl_str_p, psnl_str_p_len);
    cap_drbg_write_ram_psnl_str_p(0, psnl_str_p, psnl_str_p_len, true);

    ////////////// TRNG
    val = 0;
    val |= (1 << 8); //reset
    cap_trng_set_ctl_reg(0, val, true);
    cap_trng_set_clk_div(0, 1, true);
    cap_trng_set_init_wait_val(0, 80, true);
    cap_trng_set_swoff_tmrval(0, 0, true);
    cap_trng_set_key0(0, 0x12345678, true);
    cap_trng_set_key1(0, 0xabcdef07, true);
    cap_trng_set_key2(0, 0x87654321, true);
    cap_trng_set_key3(0, 0x1212abab, true);
    cap_trng_set_ctl_reg(0, 0x406F1, true);
    do {
      val = cap_trng_get_status(0, true);
        //SDK_TRACE_DEBUG("[DRBG] TRNG status = 0x%lx", val);
        val = cap_trng_get_fifo_level(0, true);
        //SDK_TRACE_DEBUG("[DRBG] TRNG fifo level = 0x%lx", val);
    } while (0); //val < 0x20);

    //////////// DRBG
    rng = 0;
    rng |= (0x1 << 0); ///NDRNG
    SDK_TRACE_DEBUG("[DRBG] rng = 0x%lx", rng);
    rng |= (0x1 << 2); ///DRNG
    SDK_TRACE_DEBUG("[DRBG] rng = 0x%lx", rng);
    rng |= (0x80 << 4); //size
    SDK_TRACE_DEBUG("[DRBG] rng = 0x%lx", rng);
    rng |= (0x0 << 29); ///TestDRNG
    SDK_TRACE_DEBUG("[DRBG] rng = 0x%lx", rng);
    rng |= ((psnl_str_p_len << 3) << 16); ///PSize
    SDK_TRACE_DEBUG("[DRBG] rng = 0x%lx", rng);
    rng |= (0x1 << 31); ///start 0
    SDK_TRACE_DEBUG("[DRBG] set rng = 0x%lx", rng);
    cap_drbg_set_ctl_rng(0, rng, true);

    SDK_TRACE_DEBUG("[DRBG] start polling gs");
    gs = 1;
    do {
        gs = cap_drbg_get_ctl_gs(0, true);
        //SDK_TRACE_DEBUG("[DRBG] continue polling gs");
    } while (gs != 0);
    SDK_TRACE_DEBUG("[DRBG] Initialization done");

    /*
     * Generate the random number once from ARM cpu, for the first use by the
     * data-path. Data-path program will generate for subsequent uses.
     * (Eventually with a timer-based DRBG producer ring infra in data-path, we'll
     * not need this).
     */
    SDK_TRACE_DEBUG("[DRBG] generate random number with buffer id 0");
    rng = 0;
    rng |= (0x1 << 0); //NDRNG
    rng |= (0x0 << 2); //DRNG
    rng |= (0x80 << 4); //size
    rng |= (0x0 << 29); //TestDRNG
    rng |= (0x1 << 31); //start
    SDK_TRACE_DEBUG("[DRBG] set rng = 0x%llx", rng);
    cap_drbg_set_ctl_rng(0, rng, true);

    SDK_TRACE_DEBUG("[DRBG] start polling gs");
    gs = 1;
    do {
        gs = cap_drbg_get_ctl_gs(0, true);
        //SDK_TRACE_DEBUG("[DRBG] continue polling gs");
    } while (gs != 0);
    SDK_TRACE_DEBUG("[DRBG] generate random number complete!");

    cap_drbg_read_ram_rand_num0(0, num0, 512, true);
    CAPRI_BARCO_API_PARAM_HEXDUMP((char *)"Random number set 0:", (char *)num0, 512);
}

#define CAPRI_BARCO_GCM_DUMMY_INPUT_SZ      16
#define CAPRI_BARCO_GCM_DUMMY_OUTPUT_SZ     16

sdk_ret_t capri_barco_setup_dummy_gcm1_req(uint32_t key_idx)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    uint64_t                    mem_addr, curr_addr;
    uint64_t                    ilist_msg_descr_addr = 0, olist_msg_descr_addr = 0;
    uint64_t                    ilist_mem_addr = 0, olist_mem_addr = 0, auth_tag_mem_addr = 0;
    uint64_t                    iv_addr = 0, status_addr = 0, db_addr = 0;
    barco_symm_req_descriptor_t sym_req_descr; 
    barco_sym_msg_descriptor_t  ilist_msg_descr, olist_msg_descr;
    uint64_t                    gcm1_ring_base = 0, gcm1_slot_addr = 0;
    uint32_t                    idx = 0;


#define PRE_HDR "GCM1 Dummy Enc Req"
    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B,
				NULL, &mem_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR(PRE_HDR":Failed to allocate memory for composite content");
        goto cleanup;
    }
    SDK_TRACE_DEBUG(PRE_HDR":Allocated memory @ 0x%llx to support dummy GCM request descriptor", mem_addr);

    
    ilist_msg_descr_addr = curr_addr = mem_addr;
    curr_addr += sizeof(barco_sym_msg_descriptor_t);    /* 64B */

    olist_msg_descr_addr = curr_addr;
    curr_addr += sizeof(barco_sym_msg_descriptor_t);    /* 128B */

    /* 16B input */
    ilist_mem_addr = curr_addr;
    curr_addr += CAPRI_BARCO_GCM_DUMMY_INPUT_SZ;        /* 144B */
    
    /* 16B output */
    olist_mem_addr = curr_addr;
    curr_addr += CAPRI_BARCO_GCM_DUMMY_OUTPUT_SZ;       /* 160B */

    auth_tag_mem_addr = curr_addr;
    curr_addr += 16;                                    /* 176B */

    iv_addr = curr_addr;
    curr_addr += 16;                                    /* 192B */

    status_addr = curr_addr;
    curr_addr += 16;                                    /* 208B */

    db_addr = curr_addr;
    curr_addr += 16;                                    /* 224B */

    /* Setup Input list SYM MSG descriptor */
    memset(&ilist_msg_descr, 0, sizeof(ilist_msg_descr));
    ilist_msg_descr.A0_addr             = ilist_mem_addr;
    ilist_msg_descr.O0_addr_offset      = 0;
    ilist_msg_descr.L0_data_length      = CAPRI_BARCO_GCM_DUMMY_INPUT_SZ;
    ilist_msg_descr.A1_addr             = 0;
    ilist_msg_descr.O1_addr_offset      = 0;
    ilist_msg_descr.L1_data_length      = 0;
    ilist_msg_descr.A2_addr             = 0;
    ilist_msg_descr.O2_addr_offset      = 0;
    ilist_msg_descr.L2_data_length      = 0;
    ilist_msg_descr.next_address        = 0;
    ilist_msg_descr.reserved            = 0;

    if (sdk::asic::asic_mem_write(ilist_msg_descr_addr, (uint8_t*)&ilist_msg_descr, sizeof(ilist_msg_descr))) {
        SDK_TRACE_ERR(PRE_HDR":Failed to write ilist MSG Descr @ 0x%llx", (uint64_t) ilist_msg_descr_addr);
        ret = SDK_RET_INVALID_ARG;
        goto cleanup;
    }

    /* Setup Output list SYM MSG descriptor */
    memset(&olist_msg_descr, 0, sizeof(olist_msg_descr));
    olist_msg_descr.A0_addr             = olist_mem_addr;
    olist_msg_descr.O0_addr_offset      = 0;
    olist_msg_descr.L0_data_length      = CAPRI_BARCO_GCM_DUMMY_OUTPUT_SZ;
    olist_msg_descr.A1_addr             = 0;
    olist_msg_descr.O1_addr_offset      = 0;
    olist_msg_descr.L1_data_length      = 0;
    olist_msg_descr.A2_addr             = 0;
    olist_msg_descr.O2_addr_offset      = 0;
    olist_msg_descr.L2_data_length      = 0;
    olist_msg_descr.next_address        = 0;
    olist_msg_descr.reserved            = 0;

    if (sdk::asic::asic_mem_write(olist_msg_descr_addr, (uint8_t*)&olist_msg_descr, sizeof(olist_msg_descr))) {
        SDK_TRACE_ERR(PRE_HDR":Failed to write olist MSG Descr @ 0x%llx", (uint64_t) olist_msg_descr_addr);
        ret = SDK_RET_INVALID_ARG;
        goto cleanup;
    }

    memset(&sym_req_descr, 0, sizeof(barco_symm_req_descriptor_t));
    sym_req_descr.input_list_addr       = ilist_msg_descr_addr;
    sym_req_descr.output_list_addr      = olist_msg_descr_addr;
    sym_req_descr.command               = CAPRI_BARCO_SYM_COMMAND_AES_GCM_Encrypt;
    sym_req_descr.key_descr_idx         = key_idx;
    sym_req_descr.iv_address            = iv_addr;
    sym_req_descr.auth_tag_addr         = auth_tag_mem_addr;
    sym_req_descr.header_size           = 0;
    sym_req_descr.status_addr           = status_addr;
    sym_req_descr.doorbell_addr         = db_addr;

    assert((CAPRI_BARCO_RING_SLOTS & (CAPRI_BARCO_RING_SLOTS-1)) == 0);

    gcm1_ring_base = capri_get_mem_addr(CAPRI_HBM_REG_BARCO_RING_GCM1);
    for (idx = 0; idx < CAPRI_BARCO_RING_SLOTS; idx+=8) {
        gcm1_slot_addr = gcm1_ring_base + (idx << CAPRI_BARCO_SYM_REQ_DESC_SZ_SHIFT);

        if (sdk::asic::asic_mem_write(gcm1_slot_addr, (uint8_t*)&sym_req_descr,
                    sizeof(sym_req_descr))) {
            SDK_TRACE_ERR(PRE_HDR":Failed to write Sym req descr @ 0x%llx", (uint64_t) gcm1_slot_addr);
            ret = SDK_RET_INVALID_ARG;
            goto cleanup;
        }
    }

cleanup:
    if (ret == SDK_RET_OK) {
        SDK_TRACE_DEBUG(PRE_HDR":Setup done");
    }
    return ret;
}


}    // namespace capri
}    // namespace platform
}    // namespace sdk
