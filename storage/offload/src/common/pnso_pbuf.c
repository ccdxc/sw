/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include <assert.h>
#define PNSO_ASSERT(x) assert(x)
#else
#define PNSO_ASSERT(x)
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "osal_logger.h"
#include "pnso_api.h"
#include "pnso_pbuf.h"

struct pnso_flat_buffer *
pbuf_alloc_flat_buffer(uint32_t len)
{
	struct pnso_flat_buffer *flat_buf;
	void *buf;

	flat_buf = osal_alloc(sizeof(struct pnso_flat_buffer));
	if (!flat_buf) {
		PNSO_ASSERT(0);
		goto out;
	}

	buf = osal_aligned_alloc(PNSO_MEM_ALIGN_PAGE, len);
	if (!buf) {
		PNSO_ASSERT(0);
		goto out_free;
	}

	flat_buf->buf = (uint64_t) buf;
	flat_buf->len = len;

	return flat_buf;

out_free:
	osal_free(flat_buf);
out:
	return NULL;
}

void
pbuf_free_flat_buffer(struct pnso_flat_buffer *flat_buf)
{
	void *p;

	if (!flat_buf)
		return;

	p = (void *) flat_buf->buf;
	osal_free(p);

	/* done, and let caller be responsible to free the container */
}

struct pnso_buffer_list *
pbuf_alloc_buffer_list(uint32_t count, uint32_t len)
{
	struct pnso_buffer_list *buf_list;
	struct pnso_flat_buffer *flat_buf;
	size_t num_bytes;
	uint32_t i;

	if (count <= 0) {
		PNSO_ASSERT(0);
		goto out;
	}

	num_bytes = sizeof(struct pnso_buffer_list) +
	    count * sizeof(struct pnso_flat_buffer);

	buf_list = osal_alloc(num_bytes);
	if (!buf_list) {
		PNSO_ASSERT(0);
		goto out;
	}

	for (i = 0; i < count; i++) {
		flat_buf = pbuf_alloc_flat_buffer(len);
		if (!flat_buf) {
			PNSO_ASSERT(0);
			goto out_free;
		}
		memcpy(&buf_list->buffers[i], flat_buf,
		       sizeof(struct pnso_flat_buffer));
	}
	buf_list->count = count;

	return buf_list;

out_free:
	buf_list->count = i;
	pbuf_free_buffer_list(buf_list);
out:
	return NULL;
}

void
pbuf_free_buffer_list(struct pnso_buffer_list *buf_list)
{
	struct pnso_flat_buffer *flat_buf;
	uint32_t i;

	if (!buf_list)
		return;

	for (i = 0; i < buf_list->count; i++) {
		flat_buf = &buf_list->buffers[i];
		pbuf_free_flat_buffer(flat_buf);
	}

	osal_free(buf_list);
}

struct pnso_buffer_list *
pbuf_clone_buffer_list(const struct pnso_buffer_list *src_buf_list)
{
	struct pnso_buffer_list *buf_list;
	size_t num_bytes;
	uint32_t i, count;

	if (!src_buf_list || src_buf_list->count <= 0) {
		PNSO_ASSERT(0);
		goto out;
	}

	count = src_buf_list->count;
	num_bytes = sizeof(struct pnso_buffer_list) +
	    count * sizeof(struct pnso_flat_buffer);

	buf_list = osal_alloc(num_bytes);
	if (!buf_list) {
		PNSO_ASSERT(0);
		goto out;
	}

	for (i = 0; i < count; i++)
		memcpy(&buf_list->buffers[i], &src_buf_list->buffers[i],
		       sizeof(struct pnso_flat_buffer));
	buf_list->count = count;

	return buf_list;

out:
	return NULL;
}

size_t
pbuf_get_buffer_list_len(const struct pnso_buffer_list *buf_list)
{
	struct pnso_flat_buffer *buf;
	size_t num_bytes = 0;
	uint32_t i;

	if (!buf_list)
		return num_bytes;

	for (i = 0; i < buf_list->count; i++) {
		buf = (struct pnso_flat_buffer *) &buf_list->buffers[i];
		PNSO_ASSERT(buf);

		num_bytes += buf->len;
	}

	return num_bytes;
}

bool
pbuf_is_buffer_list_sgl(const struct pnso_buffer_list *buf_list)
{
	if (!buf_list || buf_list->count == 0)
		return false;

	if (buf_list->count == 1)
		return false;

	return true;
}

void
pbuf_pprint_buffer_list(const struct pnso_buffer_list *buf_list)
{
	struct pnso_flat_buffer *flat_buf;
	uint32_t i;

	if (!buf_list)
		return;

	OSAL_LOG_INFO("buf_list: %p count: %d\n", buf_list, buf_list->count);

	for (i = 0; i < buf_list->count; i++) {
		flat_buf = (struct pnso_flat_buffer *) &buf_list->buffers[i];

		/* print only limited number of characters */
		OSAL_LOG_INFO("#%2d: flat_buf: %p len: %d buf: %.4s\n",
				i, flat_buf, flat_buf->len,
				(char *) flat_buf->buf);
	}
}
