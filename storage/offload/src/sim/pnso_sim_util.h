
/* Copy a buffer list into a flat buffer.
 * Assumes the flat buffer has been initialized with available length, or 0 for PNSO_MAX_BUFFER_LEN.
 * Return count of bytes copied.
 */
uint32_t pnso_memcpy_list_to_flat_buf(struct pnso_flat_buffer * dst,
				      const struct pnso_buffer_list *
				      src_list);

/* Copy a flat buffer into a buffer list.
 * Assumes the buffer list lengths have been filled out with allocated size, or 0 for default block size.
 * Return count of bytes copied.
 */
uint32_t pnso_memcpy_flat_buf_to_list(struct pnso_buffer_list *dst_list,
				      const struct pnso_flat_buffer *src);
