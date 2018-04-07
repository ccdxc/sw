
#include "LZR_model.h"

#include "pnso_api.h"
#include "offloader_sim.h"

#ifndef PNSO_ASSERT
extern void abort();
#define PNSO_ASSERT(x)  if (!(x)) { abort(); }
#endif

struct pnso_sim_session;

typedef struct {
    struct pnso_service_request *svc_req;
    struct pnso_service_result *svc_res;
    completion_t cb;
    void *cb_ctx;
    void *poll_fn; /* TODO */
    void *pnso_poll_ctx;
} pnso_q_request_t;

#define PNSO_SQ_DEPTH 10
typedef struct {
    volatile int head;
    volatile int tail;
    pnso_q_request_t q[PNSO_SQ_DEPTH];
} pnso_sim_q_t;

typedef struct {
    struct pnso_sim_session *sess; /* TODO */
    struct pnso_service cmd;
    struct pnso_service_status status;
    uint32_t is_first:1,
             is_last:1,
             rsvrd:30;
    struct pnso_flat_buffer input;
    struct pnso_flat_buffer output;
} pnso_sim_stage_t;

typedef struct {
    uint8_t* cmd; // scratch for commands
    uint8_t* data[2]; // for bank switching, to avoid duplicate input/output
    uint32_t data_sz; // size of each data region
} pnso_sim_scratch_region_t;

typedef int (*stage_func_t)(pnso_sim_stage_t*, void*);

struct pnso_sim_session {
    bool is_valid;
    uint32_t pad_sz;
    pnso_sim_scratch_region_t scratch;
    stage_func_t funcs[PNSO_SVC_TYPE_MAX];
};

static struct pnso_sim_session g_sessions[PNSO_MAX_SESSIONS];
static uint32_t g_session_count = 0;

static int stage_noop(pnso_sim_stage_t* stage, void* opaque);
static int stage_compress(pnso_sim_stage_t* stage, void* opaque);
static int stage_decompress(pnso_sim_stage_t* stage, void* opaque);
static int stage_encrypt(pnso_sim_stage_t* stage, void* opaque);
static int stage_decrypt(pnso_sim_stage_t* stage, void* opaque);
static int stage_pad(pnso_sim_stage_t* stage, void* opaque);
static int stage_hash(pnso_sim_stage_t* stage, void* opaque);
static int stage_chksum(pnso_sim_stage_t* stage, void* opaque);


#define CMD_SCRATCH_SZ (16 * 1024)

static int pnso_sim_init_session(struct pnso_sim_session* sess, stage_func_t *funcs,
                         uint8_t* scratch, uint32_t scratch_sz)
{
    uint32_t func_i;

    /* Use default function in case of undefined func */
    for (func_i = 0; func_i < PNSO_SVC_TYPE_MAX; func_i++) {
        /* Set stage command functions */
        sess->funcs[func_i] = funcs[func_i];
        if (sess->funcs[func_i] == NULL) {
            switch(func_i) {
            case PNSO_SVC_TYPE_COMPRESS:
                sess->funcs[func_i] = stage_compress;
                break;
            case PNSO_SVC_TYPE_DECOMPRESS:
                sess->funcs[func_i] = stage_decompress;
                break;
            case PNSO_SVC_TYPE_ENCRYPT:
                sess->funcs[func_i] = stage_encrypt;
                break;
            case PNSO_SVC_TYPE_DECRYPT:
                sess->funcs[func_i] = stage_decrypt;
                break;
            case PNSO_SVC_TYPE_PAD:
                sess->funcs[func_i] = stage_pad;
                break;
            default:
                sess->funcs[func_i] = stage_noop;
                break;
            }
        }
    }

    if (scratch_sz <= CMD_SCRATCH_SZ) {
        return -1;
    }

    sess->scratch.cmd = scratch;
    scratch += CMD_SCRATCH_SZ;
    scratch_sz -= CMD_SCRATCH_SZ;
    sess->scratch.data_sz = scratch_sz / 2;
    sess->scratch.data[0] = scratch;
    sess->scratch.data[1] = scratch + (scratch_sz / 2);

    sess->pad_sz = PNSO_DEFAULT_BLOCK_SZ;
    sess->is_valid = true;

    return 0;
}

/* Assumes scratch_sz is a power of two */
int pnso_sim_init(uint32_t sess_count, uint8_t* scratch, uint32_t scratch_sz)
{
    int rc = 0;
    uint32_t sess_i;
    stage_func_t default_funcs[PNSO_SVC_TYPE_MAX];
    uint8_t* sess_scratch;
    uint32_t sess_scratch_sz;

    memset(g_sessions, 0, sizeof(g_sessions));
    memset(default_funcs, 0, sizeof(default_funcs));

    if (sess_count > PNSO_MAX_SESSIONS) {
        return -1;
    }

    /* carve up the scratch region for use by all sessions */
    sess_scratch_sz = scratch_sz / sess_count;
    for (sess_i = 0; sess_i < sess_count; sess_i++) {
        /* Initialize each session */
        sess_scratch = scratch + (sess_scratch_sz * sess_i);
        rc = pnso_sim_init_session(&g_sessions[sess_i], default_funcs, sess_scratch, sess_scratch_sz);
        if (rc != 0) {
            break;
        }
    }

    g_session_count = sess_count;
    return rc;
}

static int stage_memcpy(pnso_sim_stage_t* stage, void* opaque)
{
    uint32_t len = stage->input.len;
    if (len > stage->output.len) {
        len = stage->output.len;
    }

    memcpy((uint8_t*)stage->input.buf, (uint8_t*)stage->output.buf, len);
    stage->output.len = len;
    return 0;
}

static int stage_noop(pnso_sim_stage_t* stage, void* opaque)
{
    // Only have to copy if last stage, since data can't stay in scratch
//    if (stage->is_last) {
//        return stage_memcpy(stage, opaque);
//    }

    stage->output = stage->input;
    return 0;
}

/* Copy a buffer list into a flat buffer.
 * Assumes the flat buffer has been initialized with available length, or 0 for PNSO_MAX_BUFFER_LEN.
 * Return count of bytes copied.
 */
static uint32_t pnso_memcpy_list_to_flat_buf(struct pnso_flat_buffer *dst,
                                             const struct pnso_buffer_list *src_list)
{
    const struct pnso_flat_buffer *src;
    uint32_t cpy_len, max_len = dst->len ? dst->len : PNSO_MAX_BUFFER_LEN;
    uint32_t buf_i;

    dst->len = 0;
    for (buf_i = 0; buf_i < src_list->count && dst->len < max_len; buf_i++) {
        src = &src_list->buffers[buf_i];
        cpy_len = src->len;
        if (dst->len + cpy_len > max_len) {
            cpy_len = max_len - dst->len;
        }
        /* TODO: use a macro for memcpy, for use in kernel and userspace */
        memcpy((uint8_t*)dst->buf + dst->len, (uint8_t*)src->buf, cpy_len);
        dst->len += cpy_len;
    }

    return dst->len;
}

/* Copy a flat buffer into a buffer list.
 * Assumes the buffer list lengths have been filled out with allocated size, or 0 for default block size.
 * Return count of bytes copied.
 */
static uint32_t pnso_memcpy_flat_buf_to_list(struct pnso_buffer_list *dst_list,
                                             const struct pnso_flat_buffer *src)
{
    struct pnso_flat_buffer *dst;
    uint32_t cpy_len, total = 0;
    uint32_t buf_i;

    for (buf_i = 0; buf_i < dst_list->count && total < src->len; buf_i++) {
        dst = &dst_list->buffers[buf_i];
        cpy_len = dst->len ? dst->len : PNSO_DEFAULT_BLOCK_SZ;
        if (cpy_len > src->len - total) {
            cpy_len = src->len - total;
        }
        /* TODO: use a macro for memcpy, for use in kernel and userspace */
        memcpy((uint8_t*)dst->buf, (uint8_t*)src->buf + total, cpy_len);
        dst->len = cpy_len;
        total += cpy_len;
    }
    dst_list->count = buf_i;

    return total;
}

static int execute_stage(struct pnso_sim_session *session, pnso_sim_stage_t *stage, void *opaque)
{
    int rc = 0;

    PNSO_ASSERT(session->funcs[stage->cmd.svc_type]);
    rc = session->funcs[stage->cmd.svc_type](stage, opaque);

    return rc;
}

void *pnso_sim_get_session(uint32_t sess_id)
{
    /* Use start index of 1 */
    if (sess_id > 0 && sess_id <= g_session_count){
        return &g_sessions[sess_id - 1];
    }
    return NULL;
}

pnso_error_t pnso_sim_execute_request(
                void *sess_handle,
                struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_t cb,
		void *cb_ctx)
{
    struct pnso_sim_session *sess = (struct pnso_sim_session*) sess_handle;
    int rc = 0;
    size_t stage_i, scratch_bank = 0;
    pnso_sim_stage_t stages[2];
    pnso_sim_stage_t *cur_stage = &stages[0];
    pnso_sim_stage_t *prev_stage = NULL;

    PNSO_ASSERT(svc_req->num_services == svc_res->num_services);
    PNSO_ASSERT(svc_req->num_services >= 1);

    memset(stages, 0, sizeof(stages));
    stages[0].sess = stages[1].sess = sess;

    /* Setup first stage, copy input data to flat scratch buffer */
    cur_stage = &stages[0];
    cur_stage->input.buf = (uint64_t) sess->scratch.data[scratch_bank];
    cur_stage->input.len = sess->scratch.data_sz;
    pnso_memcpy_list_to_flat_buf(&cur_stage->input, svc_req->src_buf);
    cur_stage->is_first = 1;

    for (stage_i = 0; stage_i < svc_req->num_services; stage_i++) {
        /* Setup command */
        cur_stage->cmd = svc_req->svc[stage_i];
        if (cur_stage->cmd.svc_type >= PNSO_SVC_TYPE_MAX) {
            rc = -1;
            goto error;
        }
        cur_stage->status = svc_res->status[stage_i];

        /* Setup input buffer */
        if (prev_stage != NULL) {
            /* Use previous output stage */
            cur_stage->input = prev_stage->output;
            cur_stage->is_first = 0;
        }

        /* Setup output buffer */
        if (cur_stage->input.buf == (uint64_t) sess->scratch.data[scratch_bank]) {
            /* swap scratch regions, so input and output are different */
            scratch_bank ^= 1;
        }
        cur_stage->output.buf = (uint64_t) sess->scratch.data[scratch_bank];
        cur_stage->output.len = sess->scratch.data_sz;
        if (stage_i == svc_req->num_services - 1) {
            /* Last stage */
            cur_stage->is_last = 1;
        }

        /* Execute stage */
        rc = execute_stage(sess, cur_stage, NULL /* TODO */);
        svc_res->status[stage_i].err = rc;
        if (rc != 0) {
            /* TODO: continue?? */
            goto error;
        }

    }

    /* Copy final result from scratch buffer to request buffer */
    pnso_memcpy_flat_buf_to_list(svc_req->dst_buf, &cur_stage->output);

error:
    return rc;
}

static int stage_lzrw1a(uint32_t action, pnso_sim_stage_t* stage)
{
    uint32_t dst_len = stage->sess->scratch.data_sz;

    if (lzrw1a_compress(action, stage->sess->scratch.cmd,
                        (uint8_t*)stage->input.buf, stage->input.len,
                        (uint8_t*)stage->output.buf, &dst_len, 0)) {
        stage->output.len = dst_len;
        return 0;
    }
    return -1;
}

static int stage_compress(pnso_sim_stage_t* stage, void* opaque)
{
    int rc = 0;

    switch(stage->cmd.algo_type) {
    case PNSO_COMPRESSOR_TYPE_NONE:
        rc = stage_noop(stage, opaque);
        break;
    case PNSO_COMPRESSOR_TYPE_LZRW1A:
        rc = stage_lzrw1a(COMPRESS_ACTION_COMPRESS, stage);
        break;
    default:
        rc = -1;
        break;
    }

    return rc;
}

static int stage_decompress(pnso_sim_stage_t* stage, void* opaque)
{
    int rc = 0;

    switch(stage->cmd.algo_type) {
    case PNSO_COMPRESSOR_TYPE_NONE:
        rc = stage_noop(stage, opaque);
        break;
    case PNSO_COMPRESSOR_TYPE_LZRW1A:
        rc = stage_lzrw1a(COMPRESS_ACTION_DECOMPRESS, stage);
        break;
    default:
        rc = -1;
        break;
    }

    return rc;
}

/* Modifies input buffer to add zero padding to end of last block,
 * where block size is given by stage->cmd.u.pad_size
 */
static int stage_pad(pnso_sim_stage_t* stage, void* opaque)
{
    int rc = 0;
    uint8_t* buf = (uint8_t*)stage->input.buf;
    uint32_t len = stage->input.len;
    uint32_t pad_sz = stage->sess->pad_sz;

    /* Start at page offset. Assumes pad_sz is a multiple of 2 */
    buf = (uint8_t*) ((uint64_t)buf & ~((uint64_t)pad_sz - 1));
    len += (uint8_t*)stage->input.buf - buf;

    /* Skip past initial blocks */
    while (len >= pad_sz) {
        len -= pad_sz;
        buf += pad_sz;
    }
    pad_sz -= len;
    buf += len;

    /* Copy input to output, or just use pointer magic */
    if (stage->is_last) {
        rc = stage_memcpy(stage, opaque);
    } else {
        stage->output = stage->input;
    }

    /* Pad output with zeroes up to pad_sz */
    if (pad_sz) {
        memset((uint8_t*)stage->output.buf + stage->output.len, 0, pad_sz);
        stage->output.len += pad_sz;
    }

    return rc;
}

static int stage_encrypt(pnso_sim_stage_t* stage, void* opaque)
{
    uint8_t key[] = "abcd\0"; /* TODO */

    return algo_encrypt_xts(stage->sess->scratch.cmd, key, (uint8_t*)stage->input.buf, stage->input.len,
                            (uint8_t*)stage->output.buf, &stage->output.len);
}

static int stage_decrypt(pnso_sim_stage_t* stage, void* opaque)
{
    uint8_t key[] = "abcd\0"; /* TODO */

    return algo_decrypt_xts(stage->sess->scratch.cmd, key, (uint8_t*)stage->input.buf, stage->input.len,
                            (uint8_t*)stage->output.buf, &stage->output.len);
}

static int stage_hash(pnso_sim_stage_t* stage, void* opaque)
{
    int rc = 0;

    /* TODO: iterate through blocks */

    PNSO_ASSERT(stage->status.num_tags > 0);

    switch (stage->cmd.algo_type) {
    case PNSO_HASH_TYPE_SHA2_256:
        if (!algo_sha_gen(stage->sess->scratch.cmd, stage->status.tags->hash_or_chksum,
                          (uint8_t*)stage->input.buf, stage->input.len, 256)) {
            rc = -1;
        }
        break;
    case PNSO_HASH_TYPE_SHA2_512:
        if (!algo_sha_gen(stage->sess->scratch.cmd, stage->status.tags->hash_or_chksum,
                          (uint8_t*)stage->input.buf, stage->input.len, 512)) {
            rc = -1;
        }
        break;
    default:
        rc = -1;
        break;
    }

    return rc;
}

static int stage_chksum(pnso_sim_stage_t* stage, void* opaque)
{
    int rc = 0;
    uint32_t temp32;
    uint8_t *hash_buf;

    /* TODO: iterate through blocks */

    PNSO_ASSERT(stage->status.num_tags > 0);
    hash_buf = stage->status.tags->hash_or_chksum;

    switch (stage->cmd.algo_type) {
    case PNSO_CHKSUM_TYPE_NONE:
        /* TODO */
        rc = -1;
        break;
    case PNSO_CHKSUM_TYPE_MCRC64:
        /* TODO */
        rc = -1;
        break;
    case PNSO_CHKSUM_TYPE_CRC32C:
        /* TODO */
        rc = -1;
        break;

    case PNSO_CHKSUM_TYPE_ADLER32:
        temp32 = algo_gen_adler32((uint8_t*)stage->input.buf, stage->input.len);
        /* TODO: do proper htonl conversion */
        hash_buf[0] = temp32 >> 24;
        hash_buf[1] = (temp32 >> 16) & 0xff;
        hash_buf[2] = (temp32 >> 8) & 0xff;
        hash_buf[3] = temp32 & 0xff;
        break;

    case PNSO_CHKSUM_TYPE_MADLER32:
        temp32 = algo_gen_madler((uint64_t*)stage->input.buf, stage->input.len);
        /* TODO: do proper htonl conversion */
        hash_buf[0] = temp32 >> 24;
        hash_buf[1] = (temp32 >> 16) & 0xff;
        hash_buf[2] = (temp32 >> 8) & 0xff;
        hash_buf[3] = temp32 & 0xff;
        break;

    default:
        rc = -1;
        break;
    }

    return rc;
}
