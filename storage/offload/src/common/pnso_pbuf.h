/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_PBUF_H__
#define __PNSO_PBUF_H__

#include "pnso_api.h"
#include "osal_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TODO-pbuf: Move this to pnso_globals.h/PNSO_MEM_ALIGN_BUF for 4K */
#define PNSO_MEM_ALIGN_PAGE	4096

/**
 * pbuf_alloc_flat_buffer() - allocates and returns a pointer to a
 * 'pnso_flat_buffer'.
 * @len:	[in]	specifies the number of bytes to allocate.
 *
 * Besides the memory for 'pnso_flat_buffer', this function also allocates
 * aligned memory of 'len' number of bytes for the data buffer within
 * 'pnso_flat_buffer' and accordingly initializes the structure members.
 *
 * TODO-pbuf:
 *	Must 'len' be a multiple of alignment size??
 *
 * Return Value:
 *	- a pointer to the allocated memory
 *	- NULL if the allocation fails
 *
 */
struct pnso_flat_buffer *pbuf_alloc_flat_buffer(uint32_t len);

/**
 * pbuf_free_flat_buffer() - releases memory allocated for the data buffer
 * within pnso_flat_buffer.
 * @flat_buf:	[in]	specifies the pointer to a pnso_flat_buffer.
 *
 * Upon returning from this function, caller should not use the data buffer
 * within the pnso_flat_buffer.  Also, caller is responsible to free the
 * pnso_flat_buffer itself explicitly.
 *
 * TODO-pbuf:
 *	Revisit clean-up part and comments, if any.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_free_flat_buffer(struct pnso_flat_buffer *flat_buf);

/**
 * pbuf_alloc_buffer_list() - allocates and returns a pointer to a
 * 'pnso_buffer_list'.
 * @count:	[in]	specifies the number of 'pnso_flat_buffer's to allocate.
 * @len:	[in]	specifies the number of bytes to allocate.
 *
 * Besides the memory for 'pnso_buffer_list', this function also allocates
 * memory of 'count' number of 'pnso_flat_buffer' and accordingly allocates them
 * and initializes the structure members.
 *
 * Return Value:
 *	- a pointer to the allocated memory
 *	- NULL if the allocation fails
 *
 */
struct pnso_buffer_list *pbuf_alloc_buffer_list(uint32_t count, uint32_t len);

/**
 * pbuf_free_buffer_list() - releases memory allocated for the pnso_flat_buffer
 * @buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * Upon returning from this function, caller should not use the members
 * within the pnso_buffer_list.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_free_buffer_list(struct pnso_buffer_list *buf_list);

/**
 * pbuf_clone_buffer_list() - allocates, shallow-copies, and returns a pointer
 * to pnso_buffer_list.
 * @src_buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * This function simply points to the source buffer list's data buffer in its
 * cloned buffer list.  Caller is responsible to avoid double-free cases.
 *
 * Return Value:
 *	- a pointer to the allocated memory
 *	- NULL if the allocation fails
 *
 */
struct pnso_buffer_list *pbuf_clone_buffer_list(
		const struct pnso_buffer_list *src_buf_list);

/**
 * pbuf_get_buffer_list_len() - returns the length of the data buffer within
 * pnso_buffer_list.
 * @buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * This function walks through the list of flat buffers, and sums up the overall
 * length of the buffer list.
 *
 * Return Value:
 *	- 0 if input buffer list is NULL
 *	- number of bytes of all the buffers in pnso_buffer_list
 *
 */
size_t pbuf_get_buffer_list_len(const struct pnso_buffer_list *buf_list);

/**
 * pbuf_is_buffer_list_sgl() - determines if input buffer list is sgl or not.
 * @buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * Return Value:
 *	- true if input buffer list contains more than one flat buffers.
 *	- false if input buffer list contains zero or only one flat buffer.
 *
 */
bool pbuf_is_buffer_list_sgl(const struct pnso_buffer_list *buf_list);

/**
 * pbuf_pprint_buffer_list() - prints details of the pnso_buffer_list.
 * @buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_pprint_buffer_list(const struct pnso_buffer_list *buf_list);

#ifdef __cplusplus
}
#endif

#endif	/* __PNSO_BUF_H__ */
