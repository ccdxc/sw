#include <gtest/gtest.h>
#include <vector>

#include "pnso_api.h"

extern "C" int pnso_sim_init(uint32_t sess_count, uint8_t* scratch, uint32_t scratch_sz);
extern "C" void *pnso_sim_get_session(uint32_t sess_id);
extern "C" pnso_error_t pnso_sim_execute_request(
                void *sess_handle,
                struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_t cb,
		void *cb_ctx);


using namespace std;

#define PNSO_TEST_SESSION_MEM (1024 * 1024)

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

    uint8_t session_mem[PNSO_TEST_SESSION_MEM];
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

TEST_F(pnso_sim_test, one_request) {
    int rc;
    size_t alloc_sz;
    void *sess;
    struct pnso_service_request *svc_req;
    struct pnso_service_result *svc_res;
    struct pnso_buffer_list *src_buflist, *dst_buflist;
    uint32_t buflist_sz;
    struct pnso_flat_buffer *buf;
    struct pnso_hash_or_chksum_tag tags[16];
    uint8_t output[16 * 4096];

    memset(tags, 0, sizeof(tags));

    /* Initialize input */
    fill_data(g_data1, sizeof(g_data1));
    fill_data(g_data2, sizeof(g_data2));

    /* Initialize session */
    rc = pnso_sim_init(2, session_mem, PNSO_TEST_SESSION_MEM);
    EXPECT_EQ(rc, 0);
    sess = pnso_sim_get_session(1);
    EXPECT_NE(sess, nullptr);

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

    /* Setup 3 services */
    svc_req->num_services = 3;
    svc_res->num_services = 3;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].req.cp_desc.threshold_len = sizeof(g_data1) - 8;

    /* Setup pad service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_PAD;

    /* Setup hash service */
    svc_req->svc[2].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[2].algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->status[2].num_tags = 16;
    svc_res->status[2].tags = tags;

    /* Execute synchronously */
    rc = pnso_sim_execute_request(sess, svc_req, svc_res, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check compression status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->status[0].err, 0);
    EXPECT_EQ(svc_res->status[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_GT(svc_res->status[0].output_data_len, 8);
    EXPECT_LT(svc_res->status[0].output_data_len, sizeof(g_data1));

    /* Check pad status */
    EXPECT_EQ(svc_res->status[1].err, 0);
    EXPECT_EQ(svc_res->status[1].svc_type, PNSO_SVC_TYPE_PAD);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);
    EXPECT_EQ(((uint8_t*)buf->buf)[4095], 0);

    /* Check hash status */
    EXPECT_EQ(svc_res->status[2].err, 0);
    EXPECT_EQ(svc_res->status[2].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_EQ(svc_res->status[2].num_tags, 1);
    EXPECT_NE(svc_res->status[2].tags->hash_or_chksum[0], 0);


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
    svc_req->svc[0].algo_type = 0; /* TODO */

    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->status[1].num_tags = 16;
    svc_res->status[1].tags = tags;

    /* Execute synchronously */
    rc = pnso_sim_execute_request(sess, svc_req, svc_res, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check encryption status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->status[0].err, 0);
    EXPECT_EQ(svc_res->status[0].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->status[0].output_data_len, sizeof(g_data1));

    /* Check hash status */
    EXPECT_EQ(svc_res->status[1].err, 0);
    EXPECT_EQ(svc_res->status[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_EQ(svc_res->status[1].num_tags, 1);
    EXPECT_NE(svc_res->status[1].tags->hash_or_chksum[0], 0);

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

    /* Setup 3 services */
    svc_req->num_services = 3;
    svc_res->num_services = 3;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].req.cp_desc.threshold_len = sizeof(g_data1) - 8;

    /* Setup pad service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_PAD;

    /* Setup encryption service */
    svc_req->svc[2].svc_type = PNSO_SVC_TYPE_ENCRYPT;
    svc_req->svc[2].algo_type = 0; /* TODO */

    /* Execute synchronously */
    rc = pnso_sim_execute_request(sess, svc_req, svc_res, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check compression status */
    EXPECT_EQ(svc_req->dst_buf->count, 1);
    EXPECT_EQ(svc_res->status[0].err, 0);
    EXPECT_EQ(svc_res->status[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_GT(svc_res->status[0].output_data_len, 8);
    EXPECT_LT(svc_res->status[0].output_data_len, sizeof(g_data1));

    /* Check pad status */
    EXPECT_EQ(svc_res->status[1].err, 0);
    EXPECT_EQ(svc_res->status[1].svc_type, PNSO_SVC_TYPE_PAD);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);
    //EXPECT_EQ(((uint8_t*)buf->buf)[4095], 0);

    /* Check encryption status */
    EXPECT_EQ(svc_res->status[2].err, 0);
    EXPECT_EQ(svc_res->status[2].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->status[2].output_data_len, 4096);

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

    /* Setup 4 services (including pad) */
    svc_req->num_services = 4;
    svc_res->num_services = 4;

    /* Setup compression service */
    svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
    svc_req->svc[0].algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
    svc_req->svc[0].req.cp_desc.threshold_len = sizeof(g_data2) - 8;

    /* Setup pad service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_PAD;

    /* Setup hash service */
    svc_req->svc[2].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[2].algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->status[2].num_tags = 16;
    svc_res->status[2].tags = tags;

    /* Setup encryption service */
    svc_req->svc[3].svc_type = PNSO_SVC_TYPE_ENCRYPT;
    svc_req->svc[3].algo_type = 0; /* TODO */

    /* Execute synchronously */
    rc = pnso_sim_execute_request(sess, svc_req, svc_res, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check compression status */
    EXPECT_EQ(svc_res->status[0].err, 0);
    EXPECT_EQ(svc_res->status[0].svc_type, PNSO_SVC_TYPE_COMPRESS);
    EXPECT_GT(svc_res->status[0].output_data_len, 8);
    EXPECT_LT(svc_res->status[0].output_data_len, sizeof(g_data2));

    /* Check pad status */
    EXPECT_EQ(svc_res->status[1].err, 0);
    EXPECT_EQ(svc_res->status[1].svc_type, PNSO_SVC_TYPE_PAD);
    buf = &svc_req->dst_buf->buffers[0];
    EXPECT_EQ(buf->len, 4096);
    //EXPECT_EQ(((uint8_t*)buf->buf)[4095], 0);

    /* Check hash status */
    EXPECT_EQ(svc_res->status[2].err, 0);
    EXPECT_EQ(svc_res->status[2].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_LE(svc_res->status[2].num_tags, 2);
    EXPECT_NE(svc_res->status[2].tags->hash_or_chksum[0], 0);

    /* Check encryption status */
    EXPECT_EQ(svc_res->status[3].err, 0);
    EXPECT_EQ(svc_res->status[3].svc_type, PNSO_SVC_TYPE_ENCRYPT);
    EXPECT_EQ(svc_res->status[3].output_data_len, 4096);

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
    svc_req->svc[0].algo_type = 0; /* TODO */

    /* Setup hash service */
    svc_req->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
    svc_req->svc[1].algo_type = PNSO_HASH_TYPE_SHA2_512;
    svc_res->status[1].num_tags = 16;
    svc_res->status[1].tags = tags;

    /* Setup decompression service */
    svc_req->svc[2].svc_type = PNSO_SVC_TYPE_DECOMPRESS;
    svc_req->svc[2].algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;

    /* Execute synchronously */
    rc = pnso_sim_execute_request(sess, svc_req, svc_res, nullptr, nullptr);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(svc_res->err, 0);

    /* Check decryption status */
    EXPECT_EQ(svc_res->status[0].err, 0);
    EXPECT_EQ(svc_res->status[0].svc_type, PNSO_SVC_TYPE_DECRYPT);
    EXPECT_EQ(svc_res->status[0].output_data_len, 4096);

    /* Check hash status */
    EXPECT_EQ(svc_res->status[1].err, 0);
    EXPECT_EQ(svc_res->status[1].svc_type, PNSO_SVC_TYPE_HASH);
    EXPECT_LE(svc_res->status[1].num_tags, 2);
    EXPECT_NE(svc_res->status[1].tags->hash_or_chksum[0], 0);

    /* Check decompression status */
    EXPECT_EQ(svc_res->status[2].err, 0);
    EXPECT_EQ(svc_res->status[2].svc_type, PNSO_SVC_TYPE_DECOMPRESS);
    EXPECT_EQ(svc_res->status[2].output_data_len, sizeof(g_data2));
    EXPECT_EQ(svc_req->dst_buf->count, 2);
    EXPECT_EQ(svc_req->dst_buf->buffers[0].len, 4096);
    EXPECT_EQ(svc_req->dst_buf->buffers[1].len, sizeof(g_data2) - 4096);
    EXPECT_EQ(memcmp((uint8_t*)svc_req->dst_buf->buffers[0].buf, g_fill_src, sizeof(g_fill_src)), 0);

    /* Restore original g_data2 for next test */
    fill_data(g_data2, sizeof(g_data2));

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
