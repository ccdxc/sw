/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "pnso_pbuf.h"

/*
 * TODO/NOTE:
 *
 * PNSO_ASSERT macro exists in current code, which is merely wrapping
 * just the assert().  Hence the following macro is named with a typo
 * deliberately, as the existing macro needs to be replaced with the
 * following eventually. Also, extend this macro further with additional
 * bells.
 *
 */
#define PNSO_ASSET(condition, msg)			\
	do {						\
		if (!(condition)) {			\
			fprintf(stdout, msg);		\
			fprintf(stdout, "\n");		\
			fflush(stdout);			\
			assert(0);			\
		}					\
	} while (0)					\

static void
ut_pbuf_alloc_flat_buffer(void)
{
	uint32_t len;
	struct pnso_flat_buffer *p;

	fprintf(stdout, "=== %s:\n", __func__);

	fprintf(stdout, "### ensure basic sanity ...\n");
	len = 1024;
	p = pbuf_alloc_flat_buffer(len);
	PNSO_ASSET(p, "Failed to allocate memory for flat buffer!");
	PNSO_ASSET(p->buf, "Failed to allocate memory for data buffer!");
	PNSO_ASSET((p->len == len), "Mismatch in data buffer length!");
	pbuf_free_flat_buffer(p);

	fprintf(stdout, "### zero length in memslign() is okay, so skip\n");
	len = 1024;
	len = 0;

	fprintf(stdout, "### ensure alignment ...\n");
	len = 13;
	p = pbuf_alloc_flat_buffer(len);
	PNSO_ASSET(p, "Failed to allocate memory for flat buffer!");
	PNSO_ASSET(((p->buf % PNSO_MEM_ALIGN_BUF) == 0),
		"Failed to align memory for data buffer!");
	pbuf_free_flat_buffer(p);
}

static void
ut_pbuf_alloc_buffer_list(void)
{
	uint32_t count, len;
	struct pnso_buffer_list *buf_list;
	struct pnso_flat_buffer *flat_buf;

	fprintf(stdout, "=== %s:\n", __func__);

	fprintf(stdout, "### ensure basic sanity ...\n");
	count = 1;
	len = 4096;
	buf_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(buf_list, "Failed to allocate memory for buffer list!");
	PNSO_ASSET((buf_list->count == 1),
			"Mismatch in buffer list countu!");
	flat_buf = &buf_list->buffers[0];
	PNSO_ASSET(flat_buf->buf,
			"Failed to allocate memory for data buffer!");
	PNSO_ASSET((flat_buf->len == len),
			"Mismatch in data buffer length!");
	pbuf_pprint_buffer_list(buf_list);
	pbuf_free_buffer_list(buf_list);

#if TODO-pbuf
	fprintf(stdout, "### zero count allocation is prevented, so skip\n");
	count = 0;
	len = 13;
	buf_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(!buf_list, "Should not allocate zero count list!");
#endif

	fprintf(stdout, "### ensure allocation of more than one count\n");
	count = 5;
	len = 13;
	buf_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(buf_list, "Failed to allocate memory for buffer list!");
	PNSO_ASSET((buf_list->count == count),
			"Mismatch in buffer list countu!");
	flat_buf = &buf_list->buffers[count-2];	/* choose a random buffer */
	PNSO_ASSET(flat_buf->buf,
			"Failed to allocate memory for data buffer!");
	PNSO_ASSET((flat_buf->len == len),
			"Mismatch in data buffer length!");
	pbuf_free_buffer_list(buf_list);
}

static void
ut_pbuf_clone_buffer_list(void)
{
	uint32_t count, len;
	struct pnso_buffer_list *src_list;
	struct pnso_buffer_list *clone_list;
	struct pnso_flat_buffer *flat_buf;
	void *s1, *s2;

	fprintf(stdout, "=== %s:\n", __func__);

	fprintf(stdout, "### ensure basic sanity ...\n");
	count = 1;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(src_list, "Failed to allocate memory for buffer list!");

	clone_list = pbuf_clone_buffer_list(src_list);
	PNSO_ASSET(clone_list, "Failed to allocate memory for buffer list!");
	PNSO_ASSET((clone_list->count == src_list->count),
			"Mismatch in buffer list countu!");
	flat_buf = &clone_list->buffers[0];
	PNSO_ASSET(flat_buf->buf,
			"Failed to allocate memory for data buffer!");
	PNSO_ASSET((flat_buf->len == len),
			"Mismatch in data buffer length!");
	pbuf_free_buffer_list(clone_list);	/* deallocates src list */

	fprintf(stdout, "### ensure data integrity ...\n");
	count = 1;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(src_list, "Failed to allocate memory for buffer list!");

	clone_list = pbuf_clone_buffer_list(src_list);
	PNSO_ASSET(clone_list, "Failed to allocate memory for buffer list!");
	PNSO_ASSET((clone_list->count == src_list->count),
			"Mismatch in buffer list countu!");

	s1 = (void *) src_list->buffers[0].buf;
	s2 = (void *) clone_list->buffers[0].buf;
	PNSO_ASSET((s1 == s2), "Mismatch in data pointers!");
	pbuf_free_buffer_list(clone_list);	/* deallocates src list */

	fprintf(stdout, "### ensure cloning with multiple flat buffers\n");
	count = 5;
	len = 13;
	src_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(src_list, "Failed to allocate memory for buffer list!");

	clone_list = pbuf_clone_buffer_list(src_list);
	PNSO_ASSET(clone_list, "Failed to allocate memory for buffer list!");
	PNSO_ASSET((clone_list->count == src_list->count),
			"Mismatch in buffer list countu!");
	flat_buf = &clone_list->buffers[count-2];
	PNSO_ASSET(flat_buf->buf,
			"Failed to allocate memory for data buffer!");
	PNSO_ASSET((flat_buf->len == len),
			"Mismatch in data buffer length!");
	pbuf_free_buffer_list(clone_list);	/* deallocates src list */
}

static void
ut_pbuf_get_buffer_list_len(void)
{
	uint32_t count, len;
	size_t size;
	struct pnso_buffer_list *src_list;

	fprintf(stdout, "=== %s:\n", __func__);

	fprintf(stdout, "### ensure length is 0 on NULL input\n");
	src_list = NULL;
	size = pbuf_get_buffer_list_len(src_list);
	PNSO_ASSET((size == 0), "Invalid data buffer length!");

	fprintf(stdout, "### ensure basic sanity ...\n");
	count = 2;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(src_list, "Failed to allocate memory for buffer list!");

	size = pbuf_get_buffer_list_len(src_list);
	PNSO_ASSET((size == 2 * len), "Mismatch in buffer lenght!");
	pbuf_free_buffer_list(src_list);
}

static void
ut_pbuf_is_buffer_list_sgl(void)
{
	uint32_t count, len;
	struct pnso_buffer_list *src_list;
	bool is_sgl;

	fprintf(stdout, "=== %s:\n", __func__);

	fprintf(stdout, "### ensure sanity on NULL input ...\n");
	src_list = NULL;
	is_sgl = pbuf_is_buffer_list_sgl(src_list);
	PNSO_ASSET((is_sgl == false), "Invalid buffer list!");

	fprintf(stdout, "### ensure basic sanity ...\n");
	count = 1;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(src_list, "Failed to allocate memory for buffer list!");

	is_sgl = pbuf_is_buffer_list_sgl(src_list);
	PNSO_ASSET((is_sgl == false),
			"Invalid count of buffers in buffer list!");
	pbuf_free_buffer_list(src_list);

	fprintf(stdout, "### ensure sgl ...\n");
	count = 2;
	len = 4096;
	src_list = pbuf_alloc_buffer_list(count, len);
	PNSO_ASSET(src_list, "Failed to allocate memory for buffer list!");

	is_sgl = pbuf_is_buffer_list_sgl(src_list);
	PNSO_ASSET((is_sgl == true),
			"Invalid count of buffers in buffer list!");
	pbuf_free_buffer_list(src_list);
}

int
main(void)
{
	ut_pbuf_alloc_flat_buffer();

	ut_pbuf_alloc_buffer_list();

	ut_pbuf_clone_buffer_list();

	ut_pbuf_get_buffer_list_len();

	ut_pbuf_is_buffer_list_sgl();

	return 0;
}
