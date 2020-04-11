//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/hal/vmotion/vmotion_msg.hpp"
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
using namespace vmotion_msg;
using vmotion_msg::VmotionMessage;

hal_ret_t
vmotion_send_msg (VmotionMessage& msg, SSL *ssl)
{
    hal_ret_t ret = HAL_RET_OK;
    int       byte_count;
    uint32_t  msg_len = msg.ByteSize() + VMOTION_MSG_HDR_LEN;
    char      *pkt = (char *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_BUFFER, msg_len);

    google::protobuf::io::ArrayOutputStream aos(pkt, msg_len);
    CodedOutputStream *coded_output = new CodedOutputStream(&aos);
    coded_output->WriteVarint32(msg.ByteSize());
    msg.SerializeToCodedStream(coded_output);

    proto_msg_dump(msg);
    if ((byte_count = SSL_write(ssl, (void *)pkt, msg_len)) <= 0) {
        HAL_TRACE_ERR("vmotion unable to send. hdr_len: {}, data_len: {}, msg_len: {} Err: {}",
                       VMOTION_MSG_HDR_LEN, msg.ByteSize(), msg_len, SSL_get_error(ssl, byte_count));
        ret = HAL_RET_ERR;
        goto end;
    }
    HAL_TRACE_DEBUG("vmotion msg send. msg_len: {} Sent: {}", msg_len, byte_count);
end:
    HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, pkt);
    free(coded_output);

    return ret;
}

hal_ret_t
vmotion_recv_msg (VmotionMessage& msg, SSL *ssl)
{
    int       byte_count;
    uint32_t  data_len, msg_len = 0, rl = 0, len, rd;
    char      *buff = (char *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_BUFFER, VMOTION_MSG_HDR_LEN);
    char      *buff_msg = NULL;

    if ((byte_count = SSL_peek(ssl, buff, VMOTION_MSG_HDR_LEN)) <= 0) {
        HAL_TRACE_ERR("vmotion msg recv error error: {}", SSL_get_error(ssl, byte_count));
        HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff);
        return HAL_RET_CONN_CLOSED;
    }

    ArrayInputStream ais(buff, VMOTION_MSG_HDR_LEN);
    CodedInputStream coded_input(&ais);
    coded_input.ReadVarint32(&data_len);

    len = msg_len = data_len + VMOTION_MSG_HDR_LEN;

    buff_msg = (char *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_BUFFER, msg_len);

    // There is no MSG_WAITALL support in SSL_read, loop till complete message is received
    while (len && ((rd = SSL_read(ssl, buff_msg + rl, len)) > 0)) {
        len -= rd;
        rl += rd;
    }

    // Not everything is read properly
    if (len) {
        HAL_TRACE_ERR("vmotion msg recv error Exp: {} Actual: {}", msg_len, (msg_len - len));
        HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff);
        HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff_msg);
        return HAL_RET_CONN_CLOSED;
    }

    HAL_TRACE_DEBUG("vmotion msg recv. msg_len: {}", msg_len);

    uint32_t tmp_data_len;
    ArrayInputStream ais_msg(buff_msg, msg_len);
    CodedInputStream coded_input_msg(&ais_msg);
    coded_input_msg.ReadVarint32(&tmp_data_len);
    CodedInputStream::Limit msg_limit = coded_input.PushLimit(msg_len);
    msg.ParseFromCodedStream(&coded_input_msg);
    coded_input.PopLimit(msg_limit);

    HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff);
    HAL_FREE(HAL_MEM_ALLOC_VMOTION_BUFFER, buff_msg);

    return HAL_RET_OK;
}

} // namespace hal
