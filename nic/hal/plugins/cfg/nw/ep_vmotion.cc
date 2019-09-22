//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/nw/ep_vmotion.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion_client.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion_server.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/proto/vmotion.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace hal {
#define VMOTION_MSG_HDR_LEN 4

using namespace google::protobuf::io;
using namespace vmotion;
using vmotion::VmotionMessage;

hal_ret_t
vmotion_send_msg(VmotionMessage& msg, int sd)
{
    int byte_count;
    uint32_t msg_len, msg_left;
    hal_ret_t ret = HAL_RET_OK;
    char *pkt;

    msg_len = msg.ByteSize() + VMOTION_MSG_HDR_LEN;
    // pkt = new char [msg_len];
    pkt = (char *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_BUFFER, msg_len);
    google::protobuf::io::ArrayOutputStream aos(pkt, msg_len);
    CodedOutputStream *coded_output = new CodedOutputStream(&aos);
    coded_output->WriteVarint32(msg.ByteSize());
    msg.SerializeToCodedStream(coded_output);

    HAL_TRACE_DEBUG("Sending:");
    proto_msg_dump(msg);
    HAL_TRACE_DEBUG("hdr_len: {}, data_len: {}, msg_len: {}", 
                    VMOTION_MSG_HDR_LEN, msg.ByteSize(), msg_len);


    if ((byte_count = send(sd, (void *)pkt, msg_len, 0)) == -1) {
        HAL_TRACE_ERR("Unable to send: {}", errno);
        ret = HAL_RET_ERR;
        goto end;
    }
    HAL_TRACE_DEBUG("Sent bytes: {}", byte_count);

end:
    HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, pkt);
    free(coded_output);

    return ret;
}

hal_ret_t
vmotion_recv_msg(VmotionMessage& msg, int sd)
{
    int byte_count;
    hal_ret_t ret = HAL_RET_OK;
    uint32_t data_len, msg_len;
    char *buff = NULL;
    char *buff_msg = NULL;

    buff = (char *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_BUFFER, 
                              VMOTION_MSG_HDR_LEN);
    if ((byte_count = recv(sd, buff, VMOTION_MSG_HDR_LEN, MSG_PEEK)) == -1) {
        HAL_TRACE_ERR("error receiving data: {}", errno);
    } else if (byte_count == 0){
        HAL_TRACE_DEBUG("client closed connection");
        HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff);
        return HAL_RET_CONN_CLOSED;
    }

    ArrayInputStream ais(buff, VMOTION_MSG_HDR_LEN);
    CodedInputStream coded_input(&ais);
    coded_input.ReadVarint32(&data_len);

    msg_len = data_len + VMOTION_MSG_HDR_LEN;

    HAL_TRACE_DEBUG("received msg hdr_len: {}, data_len: {}, msg_len: {}", 
                    VMOTION_MSG_HDR_LEN, data_len, msg_len);

    buff_msg = (char *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_BUFFER, msg_len);

    if ((byte_count = recv(sd, buff_msg, msg_len, MSG_WAITALL)) == -1) {
        HAL_TRACE_ERR("error receiving data: {}", errno);
    } else if (byte_count == 0){
        HAL_TRACE_DEBUG("client closed connection");
        HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff);
        HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff_msg);
        return HAL_RET_CONN_CLOSED;
    }
    uint32_t tmp_data_len;
    ArrayInputStream ais_msg(buff_msg, msg_len);
    CodedInputStream coded_input_msg(&ais_msg);
    coded_input_msg.ReadVarint32(&tmp_data_len);
    CodedInputStream::Limit msg_limit = coded_input.PushLimit(msg_len);
    msg.ParseFromCodedStream(&coded_input_msg);
    coded_input.PopLimit(msg_limit);

    HAL_TRACE_DEBUG("msg type: {}", VmotionMessageType_Name(msg.type()));

    HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff);
    HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff_msg);

    return ret;
}

hal_ret_t 
ep_vmotion_client (vmotion_client_t *vm_client)
{
    hal_ret_t       ret = HAL_RET_OK;

    ret = ep_vmotion_client_start(vm_client);

    // client done with vmotion

    return ret;
}

hal_ret_t 
ep_vmotion_server (int server_sd)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = ep_vmotion_server_msg();

    return ret;
}

hal_ret_t 
ep_vmotion_get_server_info(void *ctxt, string *serv_ip, 
                           uint32_t *serv_port)
{
    ep_vmotion_client_ctxt_t *client_ctxt = (ep_vmotion_client_ctxt_t *)ctxt;

    if (ctxt) {
        *serv_ip = client_ctxt->vmotion_serv_ip;
        *serv_port = client_ctxt->vmotion_serv_port;
    }

    return HAL_RET_OK;
}

bool
ep_vmotion_is_platform_type_sim(void)
{
    return g_hal_state->platform_type() == platform_type_t::PLATFORM_TYPE_SIM;
}

} // namespace hal
