#include "tls_api.hpp"
#include "ssl_helper.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/hal/src/tlscb.hpp"

namespace hal {
namespace tls {

static SSLHelper g_ssl_helper;
static hal::pd::cpupkt_ctxt_t* asesq_ctx = NULL;
//static int tcpfd = 0;
static int port = 80;

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
        asesq_ctx = hal::pd::cpupkt_ctxt_alloc_init();
        HAL_ASSERT_RETURN(asesq_ctx != NULL, HAL_RET_NO_RESOURCE);
    }

    ret = g_ssl_helper.init();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls: Failed to initialize ssl helper: {}", ret);
        return ret;
    }

    g_ssl_helper.set_send_cb(&tls_api_send_data_cb);

    //tcpfd = create_socket();
    //tls_api_start_handshake(100);

    return ret;
}

hal_ret_t
tls_api_init_flow(uint32_t qid, bool is_decrypt_flow)
{
    hal_ret_t            ret = HAL_RET_OK;
    TlsCbSpec            spec;
    TlsCbResponse        rsp;

    HAL_TRACE_DEBUG("Creating TLS Cb with for qid: {}, is_decrypt: ", qid, is_decrypt_flow);
    spec.mutable_key_or_handle()->set_tlscb_id(qid);
    spec.set_is_decrypt_flow(is_decrypt_flow);
    spec.set_other_fid(0xFFFF);
    ret = hal::tlscb_create(spec, &rsp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create tls cb with id: {}, err: {}", qid, ret);
    }
    return ret;
}

hal_ret_t
tls_api_start_handshake(uint32_t qid)
{
    // register this qid to send context
    hal::pd::cpupkt_register_tx_queue(asesq_ctx, types::WRING_TYPE_ASESQ, qid);

    return g_ssl_helper.start_connection(qid);
}

hal_ret_t
tls_api_data_receive(uint32_t qid, uint8_t* data, size_t len)
{
    return g_ssl_helper.process_nw_data(qid, data, len);
}

} // namespace tls
} // namespace hal
