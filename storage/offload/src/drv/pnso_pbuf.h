/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_PBUF_H__
#define __PNSO_PBUF_H__

#include "pnso_api.h"
#include "osal.h"

struct pnso_flat_buffer *pbuf_alloc_flat_buffer(uint32_t len);

void pbuf_free_flat_buffer(struct pnso_flat_buffer *flat_buf);

struct pnso_buffer_list *pbuf_alloc_buffer_list(uint32_t count, uint32_t len);

void pbuf_free_buffer_list(struct pnso_buffer_list *buf_list);

struct pnso_buffer_list *pbuf_clone_buffer_list(
		struct pnso_buffer_list *src_buf_list);

size_t pbuf_get_buffer_list_len(struct pnso_buffer_list *buf_list);

bool pbuf_is_buffer_list_sgl(struct pnso_buffer_list *buf_list);

void pbuf_pprint_buffer_list(struct pnso_buffer_list *buf_list);

#endif	/* __PNSO_BUF_H__ */
