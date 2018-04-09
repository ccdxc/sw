/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_global.h"
#include "pnso_logger.h"
#include "pnso_osal.h"

#include "pnso_api.h"
#include "pnso_pbuf.h"

struct pnso_flat_buffer *
pbuf_alloc_flat_buffer(uint32_t len)
{
	struct pnso_flat_buffer *flat_buf;
	void *buf;
	static int ch = 'A';	/* TODO: remove this */

	flat_buf = pnso_malloc(sizeof(struct pnso_flat_buffer));
	if (!flat_buf) {
		assert(0);
		goto out;
	}

	pnso_memalign(PNSO_MEM_ALIGN_BUF, (sizeof(char) * len), &buf);
	if (!buf) {
		assert(0);
		goto out_free;
	}

	memset(buf, ch, len);
	ch++;

	flat_buf->buf = (uint64_t) buf;
	flat_buf->len = len;

	return flat_buf;

out_free:
	pnso_free(flat_buf);
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
	pnso_free(p);

	pnso_free(flat_buf);
}

struct pnso_buffer_list *
pbuf_alloc_buffer_list(uint32_t count, uint32_t len)
{
	struct pnso_buffer_list *buf_list;
	struct pnso_flat_buffer *flat_buf;
	size_t num_bytes;
	uint32_t i;

	num_bytes = sizeof(struct pnso_buffer_list) +
	    count * sizeof(struct pnso_flat_buffer);

	buf_list = pnso_malloc(num_bytes);
	if (!buf_list) {
		assert(0);
		goto out;
	}

	for (i = 0; i < count; i++) {
		flat_buf = pbuf_alloc_flat_buffer(len);
		if (!flat_buf) {
			assert(0);
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

	pnso_free(buf_list);
}

struct pnso_buffer_list *
pbuf_clone_buffer_list(struct pnso_buffer_list *src_buf_list)
{
	struct pnso_buffer_list *buf_list;
	struct pnso_flat_buffer *flat_buf;
	size_t num_bytes;
	uint32_t i, count, len;

	if (!src_buf_list || src_buf_list->count == 0) {
		assert(0);
		goto out;
	}

	count = src_buf_list->count;
	num_bytes = sizeof(struct pnso_buffer_list) +
	    count * sizeof(struct pnso_flat_buffer);

	buf_list = pnso_malloc(num_bytes);
	if (!buf_list) {
		assert(0);
		goto out;
	}

	for (i = 0; i < count; i++) {
		len  = src_buf_list->buffers[i].len;
		flat_buf = pbuf_alloc_flat_buffer(len);
		if (!flat_buf) {
			assert(0);
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

size_t
pbuf_get_buffer_list_len(struct pnso_buffer_list *buf_list)
{
	struct pnso_flat_buffer *buf;
	size_t num_bytes = 0;
	uint32_t i;

	if (!buf_list)
		return num_bytes;

	for (i = 0; i < buf_list->count; i++) {
		buf = &buf_list->buffers[i];
		assert(buf);

		num_bytes += buf->len;
	}

	return num_bytes;
}

bool
pbuf_is_buffer_list_sgl(struct pnso_buffer_list *buf_list)
{
	if (!buf_list || buf_list->count == 0) {
		assert(0);
		return false;
	}

	if (buf_list->count == 1)
		return false;

	return true;
}

void
pbuf_pprint_buffer_list(struct pnso_buffer_list *buf_list)
{
	struct pnso_flat_buffer *flat_buf;
	uint32_t i;

	if (!buf_list)
		return;

	PNSO_LOG_INFO(PNSO_OK, "buf_list: %p count: %d",
			buf_list, buf_list->count);
	for (i = 0; i < buf_list->count; i++) {
		flat_buf = &buf_list->buffers[i];

		/* print only limited number of characters */
		PNSO_LOG_INFO(PNSO_OK, "#%2d: flat_buf: %p len: %d buf: %.4s",
			      i, flat_buf, flat_buf->len, flat_buf->buf);
	}
}

