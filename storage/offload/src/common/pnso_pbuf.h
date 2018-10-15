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

/**
 * pbuf_alloc_flat_buffer() - allocates and returns a pointer to a
 * 'pnso_flat_buffer'.
 * @len:	[in]	specifies the number of bytes to allocate.
 *
 * Besides the memory for 'pnso_flat_buffer', this function also allocates
 * memory of 'len' number of bytes for the data buffer within 'pnso_flat_buffer'
 * and accordingly initializes the structure members.
 *
 * Return Value:
 *	- a pointer to the allocated memory
 *	- NULL if the allocation fails
 *
 */
struct pnso_flat_buffer *pbuf_alloc_flat_buffer(uint32_t len);

/**
 * pbuf_aligned_alloc_flat_buffer() - allocates and returns a pointer to a
 * aligned 'pnso_flat_buffer'.
 * @align_size:	[in]	specifies the buffer alignment.
 * @len:	[in]	specifies the number of bytes to allocate.
 *
 * Besides the memory for 'pnso_flat_buffer', this function also allocates
 * aligned memory of 'len' number of bytes for the data buffer within
 * 'pnso_flat_buffer' and accordingly initializes the structure members.
 *
 * Return Value:
 *	- a pointer to the allocated memory
 *	- NULL if the allocation fails, or if align_size is not a power of 2
 *
 */
struct pnso_flat_buffer *pbuf_aligned_alloc_flat_buffer(uint32_t align_size,
		uint32_t len);

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
 * pbuf_aligned_alloc_buffer_list() - allocates and returns a pointer to a
 * aligned 'pnso_buffer_list'.
 * @count:	[in]	specifies the number of 'pnso_flat_buffer's to allocate.
 * @align_size:	[in]	specifies the buffer alignment.
 * @len:	[in]	specifies the number of bytes to allocate.
 *
 * Besides the memory for 'pnso_buffer_list', this function also allocates
 * aligned memory of 'count' number of 'pnso_flat_buffer' and accordingly
 * allocates them and initializes the structure members.
 *
 * Return Value:
 *	- a pointer to the allocated memory
 *	- NULL if the allocation fails
 *	- NULL if the allocation fails, or if align_size is not a power of 2
 *
 */
struct pnso_buffer_list *pbuf_aligned_alloc_buffer_list(uint32_t count,
		uint32_t align_size, uint32_t len);

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
 * pbuf_copy_buffer_list() - copies data from src buflist to dst buflist, and
 * returns the count of bytes copied.
 * @src_buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 * @dst_buf_list:	[out]	specifies the pointer to a pnso_buffer_list.
 *
 * Return Value:
 *	- count of bytes copied
 *	- 0 if the copy fails
 *
 */
uint32_t pbuf_copy_buffer_list(const struct pnso_buffer_list *src_buf_list,
			       struct pnso_buffer_list *dst_buf_list);

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
 * pbuf_convert_buffer_list_v2p() - converts virtual address of the buffer
 * within each of one of the flat buffers to its physical address.
 * @buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * This function walks through the list of flat buffers, and converts virtual
 * address of the buffers to physical address.  This function assumes the input
 * members of pnso_buffer_list is valid.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_convert_buffer_list_v2p(struct pnso_buffer_list *buf_list);

/**
 * pbuf_convert_flat_buffer_v2p() - converts virtual address of a flat buffer
 * within pnso_flat_buf to its physical address.
 * @flat_buf:	[in]	specifies the pointer to a pnso_flat_buffer.
 *
 * This function converts the virtual address of the flat buffer to physical
 * address.  This function assumes the input members of pnso_flat_buffer is
 * valid.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_convert_flat_buffer_v2p(struct pnso_flat_buffer *flat_buf);

/**
 * pbuf_convert_buffer_list_p2v() - converts physical address of the buffer
 * within each of one of the flat buffers to its virtual address.
 * @buf_list:	[in]	specifies the pointer to a pnso_buffer_list.
 *
 * This function walks through the list of flat buffers, and converts physical
 * address of the buffers to virtual address.  This function assumes the input
 * members of pnso_buffer_list is valid.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_convert_buffer_list_p2v(struct pnso_buffer_list *buf_list);

/**
 * pbuf_convert_flat_buffer_p2v() - converts physical address of a flat buffer
 * within pnso_flat_buf to its virtual address.
 * @flat_buf:	[in]	specifies the pointer to a pnso_flat_buffer.
 *
 * This function converts the physical address of the flat buffer to virtual
 * address.  This function assumes the input members of pnso_flat_buffer is
 * valid.
 *
 * Return Value:
 *	None
 *
 */
void pbuf_convert_flat_buffer_p2v(struct pnso_flat_buffer *flat_buf);

/**
 * pbuf_get_flat_buffer_block_count() - returns the number of blocks in a flat
 * buffer, based on total length of the flat buffer.
 * @flat_buf:	[in]	specifies the pointer to a pnso_flat_buffer.
 * @block_size:	[in]	specifies the maximum length of a block in bytes.
 *
 * This function computes the number of blocks in a flat buffer using the total
 * length of flat buffer. This function assumes the input members of
 * pnso_flat_buffer is valid.
 *
 * Return Value:
 *	- number of blocks in a flat buffer
 *
 */
uint32_t pbuf_get_flat_buffer_block_count(
		const struct pnso_flat_buffer *flat_buf, uint32_t block_size);

/**
 * pbuf_get_flat_buffer_block_len() - returns the length of data of a specified
 * block, based on total length of the flat buffer.
 * @flat_buf:	[in]	specifies the pointer to a pnso_flat_buffer.
 * @block_idx:	[in]	specifies the position/index of a block within the flat
 *			buffer for which the length of data need to be found.
 * @block_size:	[in]	specifies the maximum length of a block in bytes.
 *
 * This function computes the number of bytes as length of data in a requested
 * block, using the size of block and total length of the flat buffer. This
 * function assumes the input members of pnso_flat_buffer is valid.
 *
 * Return Value:
 *	- number of bytes within a specified block
 *
 */
uint32_t pbuf_get_flat_buffer_block_len(const struct pnso_flat_buffer *flat_buf,
		uint32_t block_idx, uint32_t block_size);

/**
 * pbuf_pad_flat_buffer_with_zeros() - pads the flat buffer with zeros from the
 * actual buffer length till the remainder of the block.
 * @flat_buf:	[in]	specifies the pointer to a pnso_flat_buffer.
 * @block_size:	[in]	specifies the maximum length of a block in bytes.
 *
 * This function computes the number of bytes to right-pad with zeros till the
 * end of the block in the flat buffer. This function assumes the input members
 * of pnso_flat_buffer is valid.
 *
 * Return Value:
 *	- number of bytes padded with zeros
 *
 */
uint32_t pbuf_pad_flat_buffer_with_zeros(struct pnso_flat_buffer *flat_buf,
		uint32_t block_size);

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
