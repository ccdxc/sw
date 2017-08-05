#include <iostream>
#include <memory.h>
#include <assert.h>
#include <zmq.h>
#include <sys/stat.h>
#include <algorithm>
#include <map>
#include <climits>
#include "buf_hdr.h"

using namespace std;

typedef enum {
    MOCK_MODEL_OPCODE_NONE      = 0,
    MOCK_MODEL_OPCODE_LOOPBACK  = 1,
    MOCK_MODEL_OPCODE_DROP      = 2,
    MOCK_MODEL_OPCODE_REWRITE   = 3,
    MOCK_MODEL_OPCODE_REPLICATE = 4,
    MOCK_MODEL_OPCODE_TO_RING = 5,
    MOCK_MODEL_OPCODE_TO_SPLIT_BUFF_75 = 6,
    MOCK_MODEL_OPCODE_TO_SPLIT_BUFF_75_REWRITE_1 = 7,
    MOCK_MODEL_OPCODE_TO_SPLIT_DESC_1_BUFF_75 = 8,
    MOCK_MODEL_OPCODE_TO_SPLIT_DESC_1_BUFF_75_REWRITE_1 = 9,
} mock_model_opcode_t;

typedef enum {
    MOCK_MODEL_MEM_OPCODE_NONE = 0,
    MOCK_MODEL_MEM_OPCODE_BUFFER = 1,
    MOCK_MODEL_MEM_OPCODE_DESCRIPTROR = 2,
} mock_model_mem_opcode_t;

#define MAX_PORTS           8
#define MAX_OUTPUT_PACKETS  128
buffer_hdr_t    *gl_output[MAX_PORTS+1][MAX_OUTPUT_PACKETS];
uint32_t        gl_num_output_pkts;

#define MAX_BUFFERS          64
buffer_hdr_t *gl_buffers[MAX_BUFFERS];
uint32_t gl_rx_buffer_index = MAX_BUFFERS - 1;


#define MAX_RINGS           8
#define MAX_DESCRIPTORS     64
buffer_hdr_t *gl_rings[MAX_RINGS][MAX_DESCRIPTORS];

//Keep track of current produced.
uint32_t gl_rings_index[MAX_RINGS];

buffer_hdr_t*
mock_model_alloc_buffer ()
{
    buffer_hdr_t *buf = (buffer_hdr_t *) malloc(4096);
    memset(buf, 0, 4096);
    return buf;
}

#define COPY_BUFFER(_dst, _src) \
        memcpy(_dst, _src, _src->size + sizeof(buffer_hdr_t));


void mock_model_memory_write(buffer_hdr_t_ *buf) {
    uint64_t addr = buf->addr;
    uint16_t type = addr & 0xffff;
    uint16_t id = (addr >> 16) & 0xffff;
    uint16_t ring_id = (addr >> 32) & 0xffff;
    buffer_hdr_t **gl;

    switch (type) {
        case MOCK_MODEL_MEM_OPCODE_BUFFER:
            assert(id < MAX_BUFFERS);
            gl = gl_buffers;
            break;
        case MOCK_MODEL_MEM_OPCODE_DESCRIPTROR:
            assert(id < MAX_RINGS);
            gl = gl_rings[ring_id];
            gl_rings_index[ring_id]++;
            break;
        default:
            assert(0);
    }

    if (gl[id] != NULL) {
        free(gl[id]);
    }
    gl[id] = buf;

}

void mock_model_memory_read(buffer_hdr_t_ *buf) {

    uint64_t addr = buf->addr;
    uint16_t type = addr & 0xffff;
    uint16_t id = (addr >> 16) & 0xffff;
    uint16_t ring_id = (addr >> 32) & 0xffff;


    switch (type) {
        case MOCK_MODEL_MEM_OPCODE_BUFFER:
            assert(id < MAX_BUFFERS);
            if (gl_buffers[id] != NULL) {
                memcpy(buf->data, gl_buffers[id]->data, buf->size);
            } else {
                buf->size = 0;
            }
            break;
        case MOCK_MODEL_MEM_OPCODE_DESCRIPTROR:
            assert(ring_id < MAX_RINGS);
            if (gl_rings[ring_id][id] != NULL) {
                memcpy(buf->data, gl_rings[ring_id][id]->data, buf->size);
            }
            break;
        default:
            assert(0);
    }


}

void
add_output_packet (buffer_hdr_t *buff)
{
    uint8_t port = buff->data[buff->size - 2];
    assert(port <= MAX_PORTS);
    buff->type = BUFF_TYPE_GET_NEXT_PKT;
    for (int i = 0; i < MAX_OUTPUT_PACKETS; i++) {
        if (gl_output[port][i] == NULL) {
            gl_output[port][i] = buff;
            gl_num_output_pkts++;
            return;
        }
    }
    assert(0);
}

/* Add buffer to a ring specified by buffer data itself */
void add_buffer_to_ring(buffer_hdr_t *buff) {

    uint16_t ring_id = buff->data[buff->size - 2];
    ring_id = ring_id - MAX_PORTS;
    assert(ring_id <= MAX_RINGS);

    uint32_t buf_id = gl_rx_buffer_index;
    gl_buffers[buf_id] = buff;
    gl_rx_buffer_index--;

    buffer_hdr_t *desc_buf = mock_model_alloc_buffer();
    uint64_t * desc_data = (uint64_t*) (desc_buf->data);

    desc_data[0] = buf_id;
    desc_data[1] = buff->size;

    gl_rings[ring_id][gl_rings_index[ring_id]++] = desc_buf;

    return;
}

void add_buffer_to_ring_rewrite(buffer_hdr_t *buff) {
    add_buffer_to_ring(buff);
    buff->data[0] = 0xFF;
}

void add_descriptor_to_ring(uint32_t ring_id, buffer_hdr_t *desc_buf) {

    gl_rings[ring_id][gl_rings_index[ring_id]++] = desc_buf;
}

void add_buffer_to_descriptor(buffer_hdr_t *desc_buf, uint32_t buf_id,
        uint32_t size) {

    uint64_t * desc_data = (uint64_t*) (desc_buf->data);
    for (int i = 0; i < 20; i += 2) {
        if (desc_data[i] == 0 and desc_data[i + 1] == 0) {
            desc_data[i] = buf_id;
            desc_data[i + 1] = size;
            return;
        }

    }
    //Max buffer limit exceeded for descriptor.
    assert(0);
}

void add_buffer_to_ring_after_splitup(buffer_hdr_t *buff,
        uint32_t buffer_split_size,
        uint32_t num_buffs_per_desc = UINT_MAX,
        uint32_t corrupt = 0) {
    uint16_t ring_id = buff->data[buff->size - 2];
    ring_id = ring_id - MAX_PORTS;
    assert(ring_id <= MAX_RINGS);

    uint32_t rem_size = buff->size;
    uint32_t offset = 0;

    while (rem_size > 0) {
        buffer_hdr_t *desc_buf = mock_model_alloc_buffer();
        uint32_t buff_cnt = 0;
        while (rem_size > 0 && buff_cnt < num_buffs_per_desc) {
            uint32_t buf_id = gl_rx_buffer_index;
            uint32_t copy_size = std::min(rem_size, buffer_split_size);
            buffer_hdr_t *buff_1 = mock_model_alloc_buffer();
            memcpy(buff_1->data, buff->data + offset, copy_size);

            if (corrupt) {
                buff_1->data[0] = 0xff;
                corrupt -= 1;
            }
            buff_1->size = copy_size;
            gl_buffers[buf_id] = buff_1;

            gl_rx_buffer_index--;
            add_buffer_to_descriptor(desc_buf, buf_id, copy_size);
            buff_cnt++;

            offset += copy_size;
            rem_size = rem_size - copy_size;
        }
        add_descriptor_to_ring(ring_id, desc_buf);
    }
}

void add_packet_to_descriptor(buffer_hdr_t *buff) {

    uint8_t opcode = buff->data[buff->size - 1];

    switch (opcode) {
        case MOCK_MODEL_OPCODE_REWRITE: {
            add_buffer_to_ring_rewrite(buff);
            break;
        }
        case MOCK_MODEL_OPCODE_TO_SPLIT_BUFF_75: {
            add_buffer_to_ring_after_splitup(buff, 75);
            break;
        }
        case MOCK_MODEL_OPCODE_TO_SPLIT_BUFF_75_REWRITE_1: {
            add_buffer_to_ring_after_splitup(buff, 75, UINT_MAX, 1);
            break;
        }
        case MOCK_MODEL_OPCODE_TO_SPLIT_DESC_1_BUFF_75: {
            add_buffer_to_ring_after_splitup(buff, 75, 1, 0);
            break;
        }
        case MOCK_MODEL_OPCODE_TO_SPLIT_DESC_1_BUFF_75_REWRITE_1: {
            add_buffer_to_ring_after_splitup(buff, 75, 1, 1);
            break;
        }
        default:
            add_buffer_to_ring(buff);
    }

}

void
rewrite_output_packet (buffer_hdr_t *buff)
{
    buff->data[0] = 0xFF;
    add_output_packet(buff);
    return;
}

void
mock_model_step_packet (buffer_hdr_t *buff)
{
    uint8_t opcode = buff->data[buff->size - 1];
    switch (opcode) {
        case MOCK_MODEL_OPCODE_NONE:
        case MOCK_MODEL_OPCODE_LOOPBACK:
            add_output_packet(buff);
            break;
        case MOCK_MODEL_OPCODE_DROP:
            break;
        case MOCK_MODEL_OPCODE_REWRITE:
            rewrite_output_packet(buff);
            break;
        case MOCK_MODEL_OPCODE_REPLICATE:
        {
            buffer_hdr_t *new_buff = mock_model_alloc_buffer();
            COPY_BUFFER(new_buff, buff);
            add_output_packet(buff);
            add_output_packet(new_buff);
            break;
        }
        case MOCK_MODEL_OPCODE_TO_RING:
            add_packet_to_descriptor(buff);
            break;

    }
    return;
}


void
mock_model_get_output_packet (buffer_hdr_t *buff)
{
    buffer_hdr_t    *out_buff = NULL;

    if (gl_num_output_pkts == 0) {
        buff->type = BUFF_TYPE_STATUS;
        buff->status = -1;
        return;
    }

    for (int port = 0; port < MAX_PORTS; port++) {
        for (int pidx = 0; pidx < MAX_OUTPUT_PACKETS; pidx++) {
            if (gl_output[port][pidx]) {
                out_buff = gl_output[port][pidx];
                gl_output[port][pidx] = NULL;
                gl_num_output_pkts--;
                COPY_BUFFER(buff, out_buff);
                return;
            }
        }
    }

    assert(0);
    return;
}




void doorbell(buffer_hdr_t_ *buf) {

    uint32_t *data = (uint32_t*) (buf->data);
    uint32_t ring_id = data[0];
    uint32_t p_idx = data[1];
    uint64_t buf_id = 0;
    uint64_t buf_size = 0;

    assert(ring_id < MAX_RINGS);
    assert(p_idx <= gl_rings_index[ring_id]);
    assert(p_idx <= MAX_DESCRIPTORS);


    for (int i = 0; i < p_idx; i++) {
        buffer_hdr_t * desc = gl_rings[ring_id][i];
        uint64_t *desc_data = (uint64_t*) desc->data;

        buf_id = desc_data[0];
        buf_size = desc_data[1];

        buffer_hdr_t *pkt_buff = mock_model_alloc_buffer();
        pkt_buff->size = buf_size;
        memcpy(pkt_buff->data, gl_buffers[buf_id]->data, buf_size);

        uint8_t dest = pkt_buff->data[pkt_buff->size - 2];
        if (dest < MAX_PORTS) {
            /* Send the packet to the port */
            pkt_buff->port = pkt_buff->data[pkt_buff->size - 2];
            add_output_packet(pkt_buff);
        } else if (dest - MAX_PORTS <= sizeof(gl_rings)) {
            /* Have to post a descriptor for this packet */
            add_packet_to_descriptor(pkt_buff);
        }
    }
gl_rings_index[ring_id] = 0;
}

void
mock_model_process_buff (buffer_hdr_t *buff)
{
    switch (buff->type) {
        case BUFF_TYPE_STEP_PKT:
            return mock_model_step_packet(buff);
        case BUFF_TYPE_GET_NEXT_PKT:
            return mock_model_get_output_packet(buff);
        case BUFF_TYPE_REG_READ:
        case BUFF_TYPE_REG_WRITE:
        case BUFF_TYPE_MEM_WRITE:
            return mock_model_memory_write(buff);
        case BUFF_TYPE_MEM_READ:
            return mock_model_memory_read(buff);
        case BUFF_TYPE_DOORBELL:
            return doorbell(buff);
        case BUFF_TYPE_STATUS:
            buff->type = BUFF_TYPE_STATUS;
            buff->status = 0;
            break;
        default:
            assert(0);
            break;
    }
    return;
}



int main (int argc, char ** argv)
{
    char tmpdir[100];
    char zmqsockstr[100];
    struct stat st = {0};
    buffer_hdr_t *buff;

    const char* user_str = std::getenv("USER");
    snprintf(tmpdir, 100, "/tmp/%s", user_str);
    if (stat(tmpdir, &st) == -1) {
        mkdir(tmpdir, 0700);
    }
    snprintf(zmqsockstr, 100, "ipc:///%s/zmqsock", tmpdir);

    //  ZMQ Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind(responder, zmqsockstr);
    assert (rc == 0);

    printf("Mock Model initialized! Waiting for pkts/command....\n");
    while (1) {
        buff = mock_model_alloc_buffer();
        assert(buff);
        zmq_recv (responder, buff, 4096, 0);
        mock_model_process_buff(buff);
        zmq_send (responder, buff, 4096, 0);
    }
    return 0;
}

