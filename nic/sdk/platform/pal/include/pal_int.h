/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef _PAL_INT_H_
#define _PAL_INT_H_

#include <stdint.h>
#include <stdbool.h>

/* *Physical* address. The compiler can't tell us this because it only
 * deals with virtual addresses */
typedef uint64_t paddr_t;

/* Number of device register areas that are mapped */
#define	MAX_UIO_MAPS	5

struct mapping_info {
	size_t	offset;
	paddr_t paddr; 
	size_t	devsize;
	void	*vaddr;
};

/*
 * struct pal_int - holds data for the PAL interface to device interrupts
 * mapping_info:	Has data for device memory mapped into user space
 * n:			The value of <n> for device files /dev/uio<n>
 * n_mappings:		Number of mappings in mapping_info
 * enable_mask:		Bits to enable/disable the interrupt(s)
 * csr_paddr:		Physical address of the CSR
 * root_enable_mask:	Mask for checking whether the interrupt is one we
 *			can handle
 * root_csr_paddr:	Physical of the address of the register for checking
 *			whether the interrupt is one we can handle
 * have_int_enable:	True of the end function should re-enable the IRQ
 * devname:		Name of the /dev/uio<n> file
 * uio_dir_fd:		File descriptor of the /sys/class/uio directory
 * fd:			Open file descriptor for reading and writing device
 *			registers
 */
struct pal_int {
	struct mapping_info	mapping_info[MAX_UIO_MAPS];
	unsigned		n;
	unsigned		n_mappings;
	uint32_t 		enable_mask;
	uint64_t		csr_paddr;
	uint32_t		root_enable_mask;
	uint64_t		root_csr_paddr;
	bool			have_int_enable;
	char			devname[256];
	int			uio_dir_fd;
	int			fd;
};

int pal_int_open(struct pal_int *pal_int, const char *name);
int pal_int_open_msi(struct pal_int *pal_int, uint64_t *msgaddr,
	uint32_t *msgdata);
int pal_int_close(struct pal_int *pal_int);
int pal_int_start(struct pal_int *pal_int);
int pal_int_end(struct pal_int *pal_int);
int pal_int_fd(struct pal_int *pal_int);
#endif
