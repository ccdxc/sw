#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

//#include "pnso_api.h"
#include "storage/offload/src/sim/pnso_sim.h"
#include "storage/offload/include/pnso_wafl.h"

using namespace std;

class pnso_sim_test : public ::testing::Test {
public:

protected:

    pnso_sim_test() {
    }

    virtual ~pnso_sim_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

/* single block request */
static uint8_t g_data1[1000];

/* multi-block request */
static uint8_t g_data2[8000];

static const char g_fill_src[] = "<html><body>The quick brown fox jumps over the lazy dog</body></html>\n";

static inline void fill_data(uint8_t* data, uint32_t data_len)
{
    uint32_t copy_len;

    while (data_len > 0) {
        copy_len = data_len >= sizeof(g_fill_src) ? sizeof(g_fill_src) : data_len;
        memcpy(data, g_fill_src, copy_len);
        data += copy_len;
        data_len -= copy_len;
    }    
}

TEST_F(pnso_sim_test, sync_request) {
    int rc;
    size_t alloc_sz;
    struct pnso_init_params init_params;
    struct pnso_service_request *svc_req;
    struct pnso_service_result *svc_res;
    struct pnso_buffer_list *src_buflist, *dst_buflist;
    uint32_t buflist_sz;
    struct pnso_flat_buffer *buf;
    struct pnso_hash_or_chksum_tag tags[16];
    uint8_t output[16 * 4096];
    char xts_iv[16] = "";

    memset(&init_params, 0, sizeof(init_params));
    memset(tags, 0, sizeof(tags));

    /* Initialize input */
    fill_data(g_data1, sizeof(g_data1));
    fill_data(g_data2, sizeof(g_data2));

    /* Initialize session */
    init_params.cp_hdr_version = 1;
    init_params.per_core_qdepth = 16;
    init_params.block_size = 4096;
    rc = pnso_init(&init_params);
    EXPECT_EQ(rc, 0);
    rc = pnso_sim_thread_init();
    EXPECT_EQ(rc, 0);

    /* Initialize key store */
    char *tmp_key = nullptr;
    uint32_t tmp_key_size = 0;
    char abcd[] = "abcd";
    pnso_sim_key_store_init((uint8_t*)malloc(64*1024), 64*1024);
    pnso_set_key_desc_idx(abcd, abcd, 4, 1);
    pnso_sim_get_key_desc_idx((void**)&tmp_key, (void**)&tmp_key, &tmp_key_size, 1);
    EXPECT_EQ(tmp_key_size, 4);
    EXPECT_EQ(0, memcmp(tmp_key, "abcd", 4));

    /* Allocate request and response */
    alloc_sz = sizeof(struct pnso_service_request) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service);
    svc_req = (struct pnso_service_request*) malloc(alloc_sz);
    EXPECT_NE(svc_req, nullptr);
    memset(svc_req, 0, alloc_sz);

    alloc_sz = sizeof(struct pnso_service_result) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service_status);
    svc_res = (struct pnso_service_result*) malloc(alloc_sz);
    EXPECT_NE(svc_res, nullptr);
    memset(svc_res, 0, alloc_sz);

    /* Allocate buffer descriptors */
    buflist_sz = sizeof(struct pnso_buffer_list) + 16*sizeof(struct pnso_flat_buffer);
    src_buflist = (struct pnso_buffer_list*) malloc(buflist_sz);
    EXPECT_NE(src_buflist, nullptr);
    dst_buflist = (struct pnso_buffer_list*) malloc(buflist_sz);
    EXPECT_NE(dst_buflist, nullptr);

    /* -------------- Test 1: Compression + Hash, single block -------------- */

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data1;
    svc_req->src_buf->buffers[0].len = sizeof(g_data1);
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 2 services */
    svc_req->num_services = 2;
    svc_res->num_services = 2;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].d.cp_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].d.cp_desc.flags = PNSO_DFLAG_ZERO_PAD | PNSO_DFLAG_INSERT_HEADER;
    svc_req->svc[0].d.cp_desc.threshold_len = sizeof(g_data1) - 8;

    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].d.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->svc[1].num_tags = 16;
    svc_res->svc[1].tags = tags;

    /* Execute synchronously */
    rc = pnso_submit_request(PNSO_BATCH_REQ_NONE, svc_req, svc_res, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check compression+pad status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);
    EXPECT_EQ(((uint8_t*)buf->buf)[4095], 0);

    /* Check hash status */
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_EQ(svc_res->svc[1].num_tags, 1);
    EXPECT_NE(svc_res->svc[1].tags->hash_or_chksum[0], 0);


    /* -------------- Test 2: Encryption + Hash, single block -------------- */

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data1;
    svc_req->src_buf->buffers[0].len = sizeof(g_data1);
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 2 services */
    svc_req->num_services = 2;
    svc_res->num_services = 2;

    /* Setup encryption service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_ENCRYPT;
    svc_req->svc[0].d.crypto_desc.key_desc_idx = 1;
    svc_req->svc[0].d.crypto_desc.iv_addr = (uint64_t) xts_iv;


    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].d.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->svc[1].num_tags = 16;
    svc_res->svc[1].tags = tags;

    /* Execute synchronously */
    rc = pnso_submit_request(PNSO_BATCH_REQ_NONE, svc_req, svc_res, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check encryption status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->svc[0].output_data_len, sizeof(g_data1));

    /* Check hash status */
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_EQ(svc_res->svc[1].num_tags, 1);
    EXPECT_NE(svc_res->svc[1].tags->hash_or_chksum[0], 0);

    /* -------------- Test 3: Compression + Encryption, single block -------------- */

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data1;
    svc_req->src_buf->buffers[0].len = sizeof(g_data1);
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 2 services */
    svc_req->num_services = 2;
    svc_res->num_services = 2;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].d.cp_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].d.cp_desc.flags = PNSO_DFLAG_ZERO_PAD | PNSO_DFLAG_INSERT_HEADER;
    svc_req->svc[0].d.cp_desc.threshold_len = sizeof(g_data1) - 8;

    /* Setup encryption service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_ENCRYPT;
    svc_req->svc[1].d.crypto_desc.key_desc_idx = 1;
    svc_req->svc[1].d.crypto_desc.iv_addr = (uint64_t) xts_iv;

    /* Execute synchronously */
    rc = pnso_submit_request(PNSO_BATCH_REQ_NONE, svc_req, svc_res, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check compression+pad status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);

    /* Check encryption status */
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->svc[1].output_data_len, 4096);

    /* -------------- Test 4: Compression + Hash + Encryption, multiple blocks -------------- */

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 2;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data2;
    svc_req->src_buf->buffers[0].len = 4096;
    svc_req->src_buf->buffers[1].buf = (uint64_t) g_data2 + 4096;
    svc_req->src_buf->buffers[1].len = sizeof(g_data2) - 4096;
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 3 services */
    svc_req->num_services = 3;
    svc_res->num_services = 3;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].d.cp_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].d.cp_desc.flags = PNSO_DFLAG_ZERO_PAD | PNSO_DFLAG_INSERT_HEADER;
    svc_req->svc[0].d.cp_desc.threshold_len = sizeof(g_data2) - 8;

    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].d.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->svc[1].num_tags = 16;
    svc_res->svc[1].tags = tags;

    /* Setup encryption service */
    svc_req->svc[2].svc_type = PNSO_SVC_TYPE_ENCRYPT;
    svc_req->svc[2].d.crypto_desc.key_desc_idx = 1;
    svc_req->svc[2].d.crypto_desc.iv_addr = (uint64_t) xts_iv;

    /* Execute synchronously */
    rc = pnso_submit_request(PNSO_BATCH_REQ_NONE, svc_req, svc_res, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check compression+pad status */
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);
    //EXPECT_EQ(((uint8_t*)buf->buf)[4095], 0);

    /* Check hash status */
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_LE(svc_res->svc[1].num_tags, 2);
    EXPECT_NE(svc_res->svc[1].tags->hash_or_chksum[0], 0);

    /* Check encryption status */
    EXPECT_EQ(svc_res->svc[2].err, 0);
    EXPECT_EQ(svc_res->svc[2].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->svc[2].output_data_len, 4096);

    /* -------------- Test 5: Decryption + Hash + Decompression, multiple blocks -------------- */

    /* Use previous test data as input to new test */
    memcpy(g_data2, (uint8_t*) buf->buf, 4096);

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data2;
    svc_req->src_buf->buffers[0].len = 4096;
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 3 services */
    svc_req->num_services = 3;
    svc_res->num_services = 3;

    /* Setup decryption service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_DECRYPT;
    svc_req->svc[0].d.crypto_desc.key_desc_idx = 1;
    svc_req->svc[0].d.crypto_desc.iv_addr = (uint64_t) xts_iv;

    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].d.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->svc[1].num_tags = 16;
    svc_res->svc[1].tags = tags;

    /* Setup decompression service */
    svc_req->svc[2].svc_type = PNSO_SVC_TYPE_DECOMPRESS;
    svc_req->svc[2].d.dc_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[2].d.dc_desc.flags = PNSO_DFLAG_HEADER_PRESENT;

    /* Execute synchronously */
    rc = pnso_submit_request(PNSO_BATCH_REQ_NONE, svc_req, svc_res, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check decryption status */
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_DECRYPT);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);

    /* Check hash status */
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_LE(svc_res->svc[1].num_tags, 2);
    EXPECT_NE(svc_res->svc[1].tags->hash_or_chksum[0], 0);

    /* Check decompression status */
    EXPECT_EQ(svc_res->svc[2].err, 0);
    EXPECT_EQ(svc_res->svc[2].svc_type, PNSO_SVC_TYPE_DECOMPRESS);
    EXPECT_EQ(svc_res->svc[2].output_data_len, sizeof(g_data2));
    EXPECT_EQ(svc_req->dst_buf->count, 2);
    EXPECT_EQ(svc_req->dst_buf->buffers[0].len, 4096);
    EXPECT_EQ(svc_req->dst_buf->buffers[1].len, sizeof(g_data2) - 4096);
    EXPECT_EQ(memcmp((uint8_t*)svc_req->dst_buf->buffers[0].buf, g_fill_src, sizeof(g_fill_src)), 0);

    /* Restore original g_data2 for next test */
    fill_data(g_data2, sizeof(g_data2));


    /* Cleanup */
    pnso_sim_thread_finit();
    pnso_sim_finit();
}

static void completion_cb(void* cb_ctx, struct pnso_service_result *svc_res)
{
    int *cb_count = (int*) cb_ctx;
    (*cb_count)++;
}

TEST_F(pnso_sim_test, async_request) {
    int rc;
    int sleep_count = 0;
    int cb_count = 0;
    size_t alloc_sz;
    struct pnso_init_params init_params;
    struct pnso_service_request *svc_req;
    struct pnso_service_result *svc_res;
    struct pnso_buffer_list *src_buflist, *dst_buflist;
    uint32_t buflist_sz;
    struct pnso_flat_buffer *buf;
    struct pnso_hash_or_chksum_tag tags[16];
    uint8_t output[16 * 4096];
    char xts_iv[16] = "";

    memset(&init_params, 0, sizeof(init_params));
    memset(tags, 0, sizeof(tags));

    /* Initialize input */
    fill_data(g_data1, sizeof(g_data1));
    fill_data(g_data2, sizeof(g_data2));

    /* Initialize session */
    init_params.cp_hdr_version = 1;
    init_params.per_core_qdepth = 16;
    init_params.block_size = 4096;
    rc = pnso_init(&init_params);
    EXPECT_EQ(rc, 0);
    rc = pnso_sim_thread_init();
    EXPECT_EQ(rc, 0);

    /* Allocate request and response */
    alloc_sz = sizeof(struct pnso_service_request) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service);
    svc_req = (struct pnso_service_request*) malloc(alloc_sz);
    EXPECT_NE(svc_req, nullptr);
    memset(svc_req, 0, alloc_sz);

    alloc_sz = sizeof(struct pnso_service_result) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service_status);
    svc_res = (struct pnso_service_result*) malloc(alloc_sz);
    EXPECT_NE(svc_res, nullptr);
    memset(svc_res, 0, alloc_sz);

    /* Allocate buffer descriptors */
    buflist_sz = sizeof(struct pnso_buffer_list) + 16*sizeof(struct pnso_flat_buffer);
    src_buflist = (struct pnso_buffer_list*) malloc(buflist_sz);
    EXPECT_NE(src_buflist, nullptr);
    dst_buflist = (struct pnso_buffer_list*) malloc(buflist_sz);
    EXPECT_NE(dst_buflist, nullptr);

    /* Initialize key store */
    char *tmp_key = nullptr;
    uint32_t tmp_key_size = 0;
    char abcd[] = "abcd";
    pnso_sim_key_store_init((uint8_t*)malloc(64*1024), 64*1024);
    pnso_set_key_desc_idx(abcd, abcd, 4, 1);
    pnso_sim_get_key_desc_idx((void**)&tmp_key, (void**)&tmp_key, &tmp_key_size, 1);
    EXPECT_EQ(tmp_key_size, 4);
    EXPECT_EQ(0, memcmp(tmp_key, "abcd", 4));


    /* -------------- Async Test 1: Compression + Hash, single block -------------- */

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data1;
    svc_req->src_buf->buffers[0].len = sizeof(g_data1);
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 2 services */
    svc_req->num_services = 2;
    svc_res->num_services = 2;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].d.cp_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].d.cp_desc.flags = PNSO_DFLAG_ZERO_PAD | PNSO_DFLAG_INSERT_HEADER;
    svc_req->svc[0].d.cp_desc.threshold_len = sizeof(g_data1) - 8;

    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].d.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->svc[1].num_tags = 16;
    svc_res->svc[1].tags = tags;

    /* Start worker thread */
    //    pnso_sim_start_worker_thread();

    /* Submit async request */
    void* poll_ctx;
    pnso_poll_fn_t poller;
    cb_count = 0;
    rc = pnso_submit_request(PNSO_BATCH_REQ_FLUSH, svc_req, svc_res, completion_cb, &cb_count, &poller, &poll_ctx);
    EXPECT_EQ(rc, 0);

    /* TODO: add gtest for batching + async */

    /* Poll for completion */
    sleep_count = 0;
    while (EAGAIN == (rc = poller(poll_ctx))) {
        usleep(1);
        sleep_count++;
    }
    EXPECT_EQ(rc, 0);
    EXPECT_GT(sleep_count, 1);
    EXPECT_LT(sleep_count, 1000);

    /* Check that callback was called exactly once */
    EXPECT_EQ(cb_count, 1);

    /* Check compression+pad status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);
    EXPECT_EQ(((uint8_t*)buf->buf)[4095], 0);

    /* Check hash status */
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_EQ(svc_res->svc[1].num_tags, 1);
    EXPECT_NE(svc_res->svc[1].tags->hash_or_chksum[0], 0);


    /* -------- Async Test 2: Encryption (of compacted block) -------- */

    /* Initialize compacted block */
    wafl_packed_blk_t *wafl = (wafl_packed_blk_t*) g_data1;
    memset(wafl, 0, sizeof(*wafl));
    wafl->wpb_hdr.wpbh_magic = WAFL_WPBH_MAGIC;
    wafl->wpb_hdr.wpbh_version = WAFL_WPBH_VERSION;
    wafl->wpb_hdr.wpbh_num_objs = 4;
    uint16_t wafl_data_offset = 1024;
    for (uint16_t i = 0; i < wafl->wpb_hdr.wpbh_num_objs; i++) {
        wafl_packed_data_info_t *wafl_data = &wafl->wpb_data_info[i];
        memset(wafl_data, 0, sizeof(*wafl_data));
        wafl_data->wpd_vvbn = 0xabc0 | i;
        wafl_data->wpd_off = wafl_data_offset + (i * 512);
        wafl_data->wpd_len = 512;
    }

    /* Initialize request buffers */
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data1;
    svc_req->src_buf->buffers[0].len = 4096;
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 1 services */
    svc_req->num_services = 1;
    svc_res->num_services = 1;

    /* Setup encryption service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_ENCRYPT;
    svc_req->svc[0].d.crypto_desc.key_desc_idx = 1;
    svc_req->svc[0].d.crypto_desc.iv_addr = (uint64_t) xts_iv;

    /* Submit async request */
    cb_count = 0;
    rc = pnso_submit_request(PNSO_BATCH_REQ_FLUSH, svc_req, svc_res, completion_cb, &cb_count, &poller, &poll_ctx);
    EXPECT_EQ(rc, 0);

    /* Poll for completion */
    sleep_count = 0;
    while (EAGAIN == (rc = poller(poll_ctx))) {
        usleep(1);
        sleep_count++;
    }
    EXPECT_EQ(rc, 0);
    EXPECT_GT(sleep_count, 0);
    EXPECT_LT(sleep_count, 1000);

    /* Check that callback was called exactly once */
    EXPECT_EQ(cb_count, 1);

    /* Check encryption status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_req->dst_buf->buffers[0].len, 4096);
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);


    /* -------- Async Test 3: Decryption + decompaction -------- */

    /* Initialize request buffers, using previous output */
    memcpy(g_data1, output, 4096);
    memset(src_buflist, 0, buflist_sz);
    memset(dst_buflist, 0, buflist_sz);
    svc_req->src_buf = src_buflist;
    svc_req->dst_buf = dst_buflist;
    svc_req->src_buf->count = 1;
    svc_req->src_buf->buffers[0].buf = (uint64_t) g_data1;
    svc_req->src_buf->buffers[0].len = 4096;
    svc_req->dst_buf->count = 16;
    for (int i = 0; i < 16; i++) {
        svc_req->dst_buf->buffers[i].buf = (uint64_t) (output + (4096 * i));
        svc_req->dst_buf->buffers[i].len = 4096;
    }

    /* Setup 2 services */
    svc_req->num_services = 2;
    svc_res->num_services = 2;

    /* Setup decryption service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_DECRYPT;
    svc_req->svc[0].d.crypto_desc.key_desc_idx = 1;
    svc_req->svc[0].d.crypto_desc.iv_addr = (uint64_t) xts_iv;

    /* Setup decompaction service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_DECOMPACT;
    svc_req->svc[1].d.decompact_desc.vvbn = 0xabc2;
    /*svc_req->svc[1].d.decompact_desc.is_uncompressed = 1;*/

    /* Submit async request */
    cb_count = 0;
    rc = pnso_submit_request(PNSO_BATCH_REQ_FLUSH, svc_req, svc_res, completion_cb, &cb_count, &poller, &poll_ctx);
    EXPECT_EQ(rc, 0);

    /* Poll for completion */
    sleep_count = 0;
    while (EAGAIN == (rc = poller(poll_ctx))) {
        usleep(1);
        sleep_count++;
    }
    EXPECT_EQ(rc, 0);
    EXPECT_GT(sleep_count, 0);
    EXPECT_LT(sleep_count, 1000);

    /* Check that callback was called exactly once */
    EXPECT_EQ(cb_count, 1);

    /* Check decryption status */
    EXPECT_EQ(svc_res->svc[0].err, 0);
    EXPECT_EQ(svc_res->svc[0].svc_type, PNSO_SVC_TYPE_DECRYPT);
    EXPECT_EQ(svc_res->svc[0].output_data_len, 4096);

    /* Check decompaction status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_req->dst_buf->buffers[0].len, 512);
    EXPECT_EQ(svc_res->svc[1].err, 0);
    EXPECT_EQ(svc_res->svc[1].svc_type, PNSO_SVC_TYPE_DECOMPACT);
    EXPECT_EQ(svc_res->svc[1].output_data_len, 512);


    /* ---- Cleanup ----- */
    //    pnso_sim_stop_worker_thread();
    pnso_sim_thread_finit();
    pnso_sim_finit();
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
