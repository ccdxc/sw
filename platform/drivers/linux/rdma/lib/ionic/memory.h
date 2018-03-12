#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdlib.h>

/** ionic_map_anon - Allocate page-aligned anonymous memory.
 * @size:	Size to allocate.
 *
 * Return: pointer to first page of memory, or NULL.
 */
void *ionic_map_anon(size_t size);

/** ionic_map_device - Map device memory via fd and offset.
 * @size:	Size to map.
 * @fd:		File descriptor representing the device context.
 * @offset:	Offset provided by the kernel as a handle for the mapping.
 *
 * The mapping may be write-only, and may use write-combining.
 *
 * Return: pointer to first page of the mapping, or NULL.
 */
void *ionic_map_device(size_t size, int fd, size_t offset);

/** ionic_unmap - Unmap anonymous or device memory.
 * @ptr:	Pointer to the first page, or NULL (do nothing).
 * @size:	Size of the map.
 */
void ionic_unmap(void *ptr, size_t size);

#endif
