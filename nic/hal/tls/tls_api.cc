#include "tls_api.hpp"
#include "ssl_helper.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/src/tls_proxy_cb.hpp"
#include "nic/hal/lkl/lklshim_tls.hpp"
#include "nic/hal/src/crypto_keys.hpp"
#include "nic/hal/src/proxyccb.hpp"

namespace hal {
namespace tls {

static SSLHelper g_ssl_helper;
static hal::pd::cpupkt_ctxt_t* asesq_ctx = NULL;
//static int tcpfd = 0;
static int port = 80;

/*
 * Global to indicate bypass mode for TLS proxy for all
 * flows, and perform tcp-proxy only.
 */
bool proxy_tls_bypass_mode = true;

#define TLS_DDOL_BYPASS_BARCO           1    /* Enqueue the request to BRQ, but bypass updating the PI of barco and 
                                              * ring BSQ doorbell 
                                              */

/**********************************************************'
 * Dummy test code. TODO: REMOVEME
 **********************************************************/
int create_socket() {
  int sockfd;
  struct sockaddr_in dest_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&(dest_addr), '\0', sizeof(dest_addr));
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(port);

  //inet_pton(AF_INET, "192.168.69.35", &dest_addr.sin_addr.s_addr);
  inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr.s_addr);

  if ( connect(sockfd, (struct sockaddr *) &dest_addr,
               sizeof(struct sockaddr_in)) == -1 ) {
    perror("Connect: ");
    exit(-1);
  }

  return sockfd;
}

// Callbacks
hal_ret_t tls_api_send_data_cb(uint32_t id, uint8_t* data, size_t len)
{
    HAL_TRACE_DEBUG("tls: tx data of len: {}", len);
    /*
    uint8_t buf[1024];
    send(tcpfd, data, len, 0);
    size_t bytes = recv(tcpfd, buf, 1024, 0);
    if(bytes <= 1024) {
        tls_api_data_receive(id, buf, bytes);
    }
    return HAL_RET_OK;
    */

    hal::pd::pd_cpupkt_send_args_t args;
    args.ctxt = asesq_ctx;
    args.type = types::WRING_TYPE_ASESQ;
    args.queue_id = id;
    args.cpu_header = NULL;
    args.p4_header = NULL;
    args.data = data;
    args.data_len = len;
    args.dest_lif = SERVICE_LIF_TCP_PROXY;
    args.qtype = 0;
    args.qid = id;
    args.ring_number = TCP_SCHED_RING_ASESQ;
    return hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, (void *)&args);
#if 0
    return hal::pd::cpupkt_send(asesq_ctx,
                                types::WRING_TYPE_ASESQ,
                                id,
                                NULL,
                                NULL,
                                data,
                                len,
                                SERVICE_LIF_TCP_PROXY,
                                0,
                                id,
                                TCP_SCHED_RING_ASESQ);
#endif
}

hal_ret_t
tls_api_program_crypto_key(char* key, uint32_t* key_index)
{
    hal_ret_t                   ret = HAL_RET_OK;
    CryptoKeyCreateRequest      create_req;
    CryptoKeyCreateResponse     create_resp;
    CryptoKeyUpdateRequest      update_req;
    CryptoKeyUpdateResponse     update_resp;

    ret = cryptokey_create(create_req, &create_resp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create crypto key: {}", ret);
        return ret;
    }

    *key_index = create_resp.keyindex();

    HAL_TRACE_DEBUG("Updating crypto key with index: {}", *key_index);
    if(!key) {
        HAL_TRACE_DEBUG("KEY IS NULL");
        return HAL_RET_INVALID_ARG;
    }

    CryptoKeySpec* spec = update_req.mutable_key();
    spec->set_keyindex(*key_index);
    spec->set_key_type(types::CRYPTO_KEY_TYPE_AES128);
    spec->set_key_size(16);
    spec->mutable_key()->assign(key, 16);
    ret = cryptokey_update(update_req, &update_resp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: failed to update key, ret: {}", ret);
        return ret;
    }
    return HAL_RET_OK;
}

hal_ret_t
tls_api_update_cb(uint32_t id,
                  uint32_t key_index,
                  uint32_t command,
                  uint32_t salt,
                  uint64_t explicit_iv,
                  bool     is_decrypt_flow,
                  uint32_t other_fid
                  )
{
    hal_ret_t           ret = HAL_RET_OK;
    TlsCbSpec           spec;
    TlsCbResponse       resp;
    TlsCbGetRequest     get_req;
    TlsCbGetResponse    get_resp;
    TlsCbGetResponseMsg resp_msg;

    // get existing values from the hw
    get_req.mutable_key_or_handle()->set_tlscb_id(id);
    ret = tlscb_get(get_req, &resp_msg);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tlscb for id: {}", id);
        return ret;
    }
    get_resp = resp_msg.response(0);
    spec.mutable_key_or_handle()->set_tlscb_id(id);
    spec.set_crypto_key_idx(key_index);
    spec.set_command(command);
    spec.set_salt(salt);
    spec.set_explicit_iv(explicit_iv);
    spec.set_is_decrypt_flow(is_decrypt_flow);
    spec.set_other_fid(other_fid);
    HAL_TRACE_DEBUG("tls: got serq ci {}", get_resp.spec().serq_ci());
    spec.set_serq_ci(get_resp.spec().serq_ci());
    spec.set_serq_pi(get_resp.spec().serq_pi());
    spec.set_l7_proxy_type(get_resp.spec().l7_proxy_type());

    HAL_TRACE_DEBUG("tls: updating TCPCB: id: {}, key_index: {}, command: {}, salt: {}, iv: {}, is_decrypt: {}",
                     id, key_index, command, salt, explicit_iv, is_decrypt_flow);
    ret = tlscb_update(spec, &resp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update tlscb for id: {}, ret: ", id, ret);
        return ret;
    }
    return HAL_RET_OK;
}

hal_ret_t
tls_api_hs_done_cb(uint32_t id, uint32_t oflowid, hal_ret_t ret, hs_out_args_t* args, bool is_v4_flow)
{
    uint32_t            enc_key_index = 0;
    uint32_t            dec_key_index = 0;

    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SSL Handshake failed, err: {}", ret);
        return ret;
    }

    // Program dec key
    ret = tls_api_program_crypto_key((char*)args->read_key, &dec_key_index);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program dec crypto key, ret {}", ret);
        return ret;
    }

    for (int i =0; i < 16; i++) {
      char buf[16] = {0};
      uint32_t dummy_key_index;
      // Program dec key
      ret = tls_api_program_crypto_key((char*)buf, &dummy_key_index);
      if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program dec crypto key, ret {}", ret);
        return ret;
      }
    }      
    // Program enc key
    ret = tls_api_program_crypto_key((char*)args->write_key, &enc_key_index);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program enc crypto key, ret {}", ret);
        return ret;
    }


    // program tlscb for decr
    ret = tls_api_update_cb(id,
                            dec_key_index,
                            0x30100000,
                            *((uint32_t*)args->read_iv),
                            *((uint64_t*)args->read_seq_num),
                            true,
                            oflowid);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: Failed to program decrypt TLSCB for id: {}, ret: {}", id, ret);
        return ret;
    }

    // program encr flow
    ret = tls_api_update_cb(oflowid,
                            enc_key_index,
                            0x30000000,
                            *((uint32_t*)args->write_iv),
                            *((uint64_t*)args->write_seq_num),
                            false,
                            id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: Failed to program TLSCB for id: {}, ret: {}", oflowid, ret);
        return ret;
    }
      
    // Inform LKL  
    if (is_v4_flow)
        lklshim_release_client_syn(id);
    else
        lklshim_release_client_syn6(id);

    return ret;
}

hal_ret_t
tls_api_createcb(uint32_t qid, bool is_decrypt_flow, uint32_t other_fid,
                 types::AppRedirType l7_proxy_type)
{
    hal_ret_t            ret = HAL_RET_OK;
    TlsCbSpec            spec;
    TlsCbResponse        rsp;

    HAL_TRACE_DEBUG("Creating TLS Cb with for qid: {}, is_decrypt: ", qid, is_decrypt_flow);
    spec.mutable_key_or_handle()->set_tlscb_id(qid);
    spec.set_is_decrypt_flow(is_decrypt_flow);

    /*
     * If TLS bypass mode is set, we'll bypass barco requests in data path.
     */
    if (proxy_tls_bypass_mode) {
        spec.set_debug_dol(TLS_DDOL_BYPASS_BARCO);
        spec.set_other_fid(other_fid);
    } else {
        spec.set_other_fid(0xFFFF);
    }
    spec.set_l7_proxy_type(l7_proxy_type);
    ret = hal::tlscb_create(spec, &rsp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create tls cb with id: {}, err: {}", qid, ret);
    }
    return ret;
}

/*************************
 * APIs
 ************************/
hal_ret_t
tls_api_init(void)
{
    hal_ret_t   ret = HAL_RET_OK;

    // Initialize pkt send ctxt
    if (asesq_ctx == NULL)   {
        hal::pd::pd_cpupkt_ctxt_alloc_init_args_t args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_ALLOC_INIT, (void *)&args);
        // asesq_ctx = hal::pd::cpupkt_ctxt_alloc_init();
        asesq_ctx = args.ctxt;
        HAL_ASSERT_RETURN(asesq_ctx != NULL, HAL_RET_NO_RESOURCE);
    }

    ret = g_ssl_helper.init();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: Failed to initialize ssl helper: {}", ret);
        return ret;
    }

    g_ssl_helper.set_send_cb(&tls_api_send_data_cb);
    g_ssl_helper.set_hs_done_cb(&tls_api_hs_done_cb);

    //tcpfd = create_socket();
    //tls_api_start_handshake(100, 101);

    return ret;
}

hal_ret_t
tls_api_init_flow(uint32_t enc_qid, uint32_t dec_qid)
{
    hal_ret_t   ret = HAL_RET_OK;
    ret = tls_api_createcb(enc_qid, false, dec_qid,
                           proxyccb_tlscb_l7_proxy_type_eval(enc_qid));
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: Failed to create enc flow tlscb, qid: {}, ret: {}", enc_qid, ret);
        return ret;
    }

    /*
     * If TLS bypass mode is set, we'll fake both flows as "encrypt", so the bypass barco
     * logic is triggered, as there is no TLS processing to be done anyway.
     */
    ret = tls_api_createcb(dec_qid, !proxy_tls_bypass_mode, enc_qid,
                           proxyccb_tlscb_l7_proxy_type_eval(dec_qid));
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: Failed to create dec flow tlscb, qid: {}, ret: {}", dec_qid, ret);
        return ret;
    }
    return ret;
}

hal_ret_t
tls_api_start_handshake(uint32_t enc_qid, uint32_t dec_qid, bool is_v4_flow)
{
    // Start handskake towards decrypt
    // register this qid to send context
    hal::pd::pd_cpupkt_register_tx_queue_args_t args;
    args.ctxt = asesq_ctx;
    args.type = types::WRING_TYPE_ASESQ;
    args.queue_id = dec_qid;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_TXQ, (void *)&args);
    // hal::pd::cpupkt_register_tx_queue(asesq_ctx, types::WRING_TYPE_ASESQ, dec_qid);

    return g_ssl_helper.start_connection(dec_qid, enc_qid, is_v4_flow);
}

hal_ret_t
tls_api_data_receive(uint32_t qid, uint8_t* data, size_t len)
{
    return g_ssl_helper.process_nw_data(qid, data, len);
}

} // namespace tls
} // namespace hal
