/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

#include "osal_logger.h"
#include "storage/offload/src/common/pnso_pbuf.h"

using namespace std;

class pnso_pbuf_test : public ::testing::Test {
public:

protected:

    pnso_pbuf_test() {
    }

    virtual ~pnso_pbuf_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_pbuf_test, ut_pbuf_alloc_flat_buffer) {
	uint32_t len;
	struct pnso_flat_buffer *p;

	OSAL_LOG_INFO("=== %s:\n", __func__);

	OSAL_LOG_INFO("### ensure basic sanity ...\n");
	len = 1024;
	p = pbuf_alloc_flat_buffer(len);
	EXPECT_NE(p, nullptr);
	EXPECT_NE(p->buf, 0);
	EXPECT_EQ(p->len, len);
	pbuf_free_flat_buffer(p);

	OSAL_LOG_INFO("### zero length in memslign() is okay, so skip\n");
	len = 1024;
	len = 0;

	OSAL_LOG_INFO("### ensure alignment ...\n");
	len = 13;
	p = pbuf_alloc_flat_buffer(len);
	EXPECT_NE(p, nullptr);
	EXPECT_EQ((p->buf % PNSO_MEM_ALIGN_PAGE), 0);
	pbuf_free_flat_buffer(p);
}

TEST_F(pnso_pbuf_test, ut_pbuf_alloc_buffer_list) {
	uint32_t count, len;
	struct pnso_buffer_list *buf_list;
	struct pnso_flat_buffer *flat_buf;

	OSAL_LOG_INFO("=== %s:\n", __func__);

	OSAL_LOG_INFO("### ensure basic sanity ...\n");
	count = 1;
	len = 4096;
	buf_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(buf_list, nullptr);
	EXPECT_EQ(buf_list->count, 1);
	flat_buf = &buf_list->buffers[0];
	EXPECT_NE(flat_buf->buf, 0);
	EXPECT_EQ(flat_buf->len, len);
	pbuf_pprint_buffer_list(buf_list);
	pbuf_free_buffer_list(buf_list);

#if TODO-pbuf
	OSAL_LOG_INFO("### zero count allocation is prevented, so skip\n");
	count = 0;
	len = 13;
	buf_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_EQ(buf_list, nullptr);
#endif

	OSAL_LOG_INFO("### ensure allocation of more than one count\n");
	count = 5;
	len = 13;
	buf_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(buf_list, nullptr);
	EXPECT_EQ(buf_list->count, count);
	flat_buf = &buf_list->buffers[count-2];	/* choose a random buffer */
	EXPECT_NE(flat_buf->buf, 0);
	EXPECT_EQ(flat_buf->len, len);
	pbuf_free_buffer_list(buf_list);
}

TEST_F(pnso_pbuf_test, ut_pbuf_clone_buffer_list) {
	uint32_t count, len;
	struct pnso_buffer_list *src_list;
	struct pnso_buffer_list *clone_list;
	struct pnso_flat_buffer *flat_buf;
	void *s1, *s2;

	OSAL_LOG_INFO("=== %s:\n", __func__);

	OSAL_LOG_INFO("### ensure basic sanity ...\n");
	count = 1;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_list, nullptr);

	clone_list = pbuf_clone_buffer_list(src_list);
	EXPECT_NE(clone_list, nullptr);
	EXPECT_EQ(clone_list->count, src_list->count);
	flat_buf = &clone_list->buffers[0];
	EXPECT_NE(flat_buf->buf, 0);
	EXPECT_EQ(flat_buf->len, len);
	pbuf_free_buffer_list(clone_list);	/* deallocates src list */

	OSAL_LOG_INFO("### ensure data integrity ...\n");
	count = 1;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_list, nullptr);

	clone_list = pbuf_clone_buffer_list(src_list);
	EXPECT_NE(clone_list, nullptr);
	EXPECT_EQ(clone_list->count, src_list->count);

	s1 = (void *) src_list->buffers[0].buf;
	s2 = (void *) clone_list->buffers[0].buf;
	EXPECT_EQ(s1, s2);
	pbuf_free_buffer_list(clone_list);	/* deallocates src list */

	OSAL_LOG_INFO("### ensure cloning with multiple flat buffers\n");
	count = 5;
	len = 13;
	src_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_list, nullptr);

	clone_list = pbuf_clone_buffer_list(src_list);
	EXPECT_NE(clone_list, nullptr);
	EXPECT_EQ(clone_list->count, src_list->count);
	flat_buf = &clone_list->buffers[count-2];
	EXPECT_NE(flat_buf->buf, 0);
	EXPECT_EQ(flat_buf->len, len);
	pbuf_free_buffer_list(clone_list);	/* deallocates src list */
}

TEST_F(pnso_pbuf_test, ut_pbuf_get_buffer_list_len) {
	uint32_t count, len;
	size_t size;
	struct pnso_buffer_list *src_list;

	OSAL_LOG_INFO("=== %s:\n", __func__);

	OSAL_LOG_INFO("### ensure length is 0 on NULL input\n");
	src_list = NULL;
	size = pbuf_get_buffer_list_len(src_list);
	EXPECT_EQ(size, 0);

	OSAL_LOG_INFO("### ensure basic sanity ...\n");
	count = 2;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_list, nullptr);

	size = pbuf_get_buffer_list_len(src_list);
	EXPECT_EQ(size, 2 * len);
	pbuf_free_buffer_list(src_list);
}

TEST_F(pnso_pbuf_test, ut_pbuf_is_buffer_list_sgl) {
	uint32_t count, len;
	struct pnso_buffer_list *src_list;
	bool is_sgl;

	OSAL_LOG_INFO("=== %s:\n", __func__);

	OSAL_LOG_INFO("### ensure sanity on NULL input ...\n");
	src_list = NULL;
	is_sgl = pbuf_is_buffer_list_sgl(src_list);
	EXPECT_EQ(is_sgl, false);

	OSAL_LOG_INFO("### ensure basic sanity ...\n");
	count = 1;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_list, nullptr);

	is_sgl = pbuf_is_buffer_list_sgl(src_list);
	EXPECT_EQ(is_sgl, false);
	pbuf_free_buffer_list(src_list);

	OSAL_LOG_INFO("### ensure sgl ...\n");
	count = 2;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_list, nullptr);

	is_sgl = pbuf_is_buffer_list_sgl(src_list);
	EXPECT_EQ(is_sgl, true);
	pbuf_free_buffer_list(src_list);
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
