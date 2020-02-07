/*
 * Copyright (c) 2019, Pensando Systems Inc.
 *
 * Functions for accessing the UIO-based interfaces while hiding the icky
 * details
 */

/*
 * Debug flag. If defined, all open()s will print the name of the file
 * if they failed
 */
#undef PRINT_OPEN_PATHNAME

#include <ctype.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <pal_int.h>
#include <pal_reg.h>

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))

#define SYSFS_UIO_DIR		"/sys/class/uio"
#define DEV_TEMPLATE		"/dev/uio"

#define	ENABLE_CSR_PADDR 	"/proc/device-tree/soc/%s/enable_csr_paddr"
#define	ENABLE_MASK 		"/proc/device-tree/soc/%s/enable_mask"

/*
 * Indicate that a write should be used to explicitly indicate end of interrupt
 * processing.
 */
#define EXPLICIT_WRITE_FOR_EOI

/*
 * Normal CSRs
 *
 * Register names follow the hardware register names
 *
 * @enable_set:		Every one bit in values written enables the
 *			corresponding interrupt
 * @enable_set:		Every one bit in values written disables the
 *			corresponding interrupt
 * @intreg:		One for each interrupt that is being asserted
 * @test:		Test register. Write a one for a particular bit
 *			asserts the corresponding interrupt
 */
struct csr {
	uint32_t	intreg;
	uint32_t	test_set;
	uint32_t	enable_set;
	uint32_t	enable_clear;
};

/*
 * Root CSR
 * @intreg:		The bit corresponding to an interrupt source is one
 *			if that interrupt is asserted.
 * @enable_rw_reg:	Write a one to enable an interrupt, zero to disable it
 * @rw_reg:		One for each asserted interrupt
 */
struct root_csr {
	uint32_t	intreg;
	uint32_t	enable_rw_reg;
	uint32_t	rw_reg;
};

#ifdef PRINT_OPEN_PATHNAME
#define print_open_pathname(rc, name)	__print_open_pathname(rc, name, \
	__FILE__, __LINE__)

static void __print_open_pathname(int rc, const char *name, const char *file,
	unsigned line)
{
	if (rc == -1)
		printf("%s:%u: open(%s) failed\n", file, line, name);
}
#else
static void print_open_pathname(int rc, const char *name)
{
}
#endif

/*
 * Using vsnprintf, print a formatted string into a buffer. Does the checks
 * for errors that rarely get done to avoid buffer overflows
 *
 * Returns 0 on success, -1 on failure.
 */
static int print_to_buffer(char *buf, size_t bufsize,
	const char *fmt, ...)
{
	va_list ap;
	ssize_t ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, bufsize, fmt, ap);
	va_end(ap);

	if (ret == 1)
		return -1;
	if ((size_t)ret >= bufsize)
		return -1;

	return 0;
}

/*
 * Read from the file /sys/class/uio/uio<n>/maps/map<map>/<filename>.
 *
 * Returns the number of bytes read on success, otherwise -1.
 */
static size_t read_file_common(int uio_dir_fd, char *buf, size_t buf_size,
	const char *fmt, va_list *ap)
{
	char file_path[2048];
	ssize_t zrc;
	int len;
	int fd;
	int rc;

	len = vsnprintf(file_path, sizeof(file_path), fmt, *ap);
	if (len < 0) {
		errno = EIO;
		return -1;
	}
	if ((unsigned)len >= sizeof(file_path)) {
		errno = EINVAL;
		return -1;
	}

	fd = openat(uio_dir_fd, file_path, O_RDONLY);
	if (fd < 0)
		return -1;

	zrc = read(fd, buf, buf_size);

	rc = close(fd);
	if (rc < 0)
		return -1;
	return zrc;
}

/*
 * Read from the file /sys/class/uio/uio<n>/maps/map<map>/<filename>.
 * The data read is expected to end with a newline, which will be overwritten
 * by a NUL character.
 *
 * Returns the number of bytes read on success, otherwise -1.
 */
static size_t read_file_text(int uio_dir_fd, char *buf, size_t buf_size,
	const char *fmt, ...)
{
	ssize_t zrc;
	va_list ap;

	va_start(ap, fmt);
	zrc = read_file_common(uio_dir_fd, buf, buf_size, fmt, &ap);
	va_end(ap);

	if (zrc > 0) {
		if (buf[zrc - 1] != '\n') {
			errno = EIO;
			return -1;
		}
		buf[zrc - 1] = '\0';
	}

	return zrc;
}

/*
 * Read from the file /sys/class/uio/uio<n>/maps/map<map>/<filename>.
 * The data read is expected to end with a newline, which will be overwritten
 * by a NUL character.
 *
 * Returns the number of bytes read on success, otherwise -1.
 */
static size_t read_file_binary(int uio_dir_fd, char *buf, size_t buf_size,
	const char *fmt, ...)
{
	ssize_t zrc;
	va_list ap;

	va_start(ap, fmt);
	zrc = read_file_common(uio_dir_fd, buf, buf_size, fmt, &ap);
	va_end(ap);

	return zrc;
}

/*
 * Looks to see if the name is "uio" followed by a numeric value, followed
 * by a NUL character
 */
static bool is_uio_name(const char *name, unsigned long *n)
{
	const char *n_str;
	char *endptr;

	if (strncmp(name, "uio", strlen("uio")) != 0)
		return false;

	n_str = name + strlen("uio");
	*n = strtoul(n_str, &endptr, 0);
	if (*n_str == '\0' || *endptr != '\0')
		return false;

	return true;
}

/*
 * Given a device number, this creates the device name.
 *
 * Returns 0 on success, otherwise -1
 */
static int gen_device_name(unsigned long n, char *devname,
	size_t devname_len)
{
	int len;

	len = snprintf(devname, devname_len, "%s%lu", DEV_TEMPLATE, n);
	if (len == -1)
		return -1;
	if ((unsigned)len >= devname_len)
		return -1;
	return 0;
}

static int __enable_mask_get(const char *path, uint32_t *enable_mask)
{
	ssize_t zrc;
	int fd;

	fd = open(path, O_RDONLY);
	print_open_pathname(fd, path);
	if (fd == -1)
		return -1;

	zrc = read(fd, enable_mask, sizeof(*enable_mask));
	if (zrc != -1) {
		if ((size_t)zrc < sizeof(*enable_mask)) {
			errno = EIO;
			return -1;
		}
	}

	if (close(fd) == -1)
		return -1;

	*enable_mask = be32toh(*enable_mask);
	return 0;
}

/*
 * Read the mask used to enable/disable interrupts from the device tree
 */

static int enable_mask_get(const char *name, uint32_t *enable_mask)
{
	char buf[1024];
	int rc;

	print_to_buffer(buf, sizeof(buf), ENABLE_MASK, name);
	rc = __enable_mask_get(buf, enable_mask);
	return rc;
}

/*
 * Read the physical address of the CSR with which we want to enable/disable
 */
static int __csr_paddr_get(const char *path, uint64_t *csr_paddr)
{
	ssize_t zrc;
	int fd;

	fd = open(path, O_RDONLY);
	print_open_pathname(fd, path);
	if (fd == -1)
		return -1;

	zrc = read(fd, csr_paddr, sizeof(*csr_paddr));
	if (zrc == -1) {
		if ((size_t)zrc < sizeof(*csr_paddr)) {
			close(fd);
			return -1;
		}
	}

	if (close(fd) == -1)
		return -1;

	*csr_paddr = be64toh(*csr_paddr);
	return 0;
}

static int csr_paddr_get(const char *name, uint64_t *paddr)
{
	char buf[1024];

	print_to_buffer(buf, sizeof(buf), ENABLE_CSR_PADDR, name);
	return __csr_paddr_get(buf, paddr);
}

static int get_device_info(const char *name, uint32_t *enable_mask,
	paddr_t *csr_paddr)
{
	int rc;

	rc = enable_mask_get(name, enable_mask);
	if (rc < 0)
		return rc;
	rc = csr_paddr_get(name, csr_paddr);
	if (rc < 0)
		return rc;
	return 0;
}

/*
 * Get the device number from the name and use it to generate
 * the name of the device in /dev
 * pal_int:	Pointer to the struct pal_int on which we are working
 * d_name:	Name read from the root UIO directory
 */
static int get_device_name(struct pal_int *pal_int, const char *d_name)
{
	gen_device_name(pal_int->n, pal_int->devname,
		sizeof(pal_int->devname));
	return 0;
}

/*
 * Try to open the given name.
 * pal_int:	Pointer to the struct pal_int we are working on
 * d_name:	Name of the UIO directory
 * name:	Name the user requested to open
 * name_data:	Contents of the name file in the UIO directory
 *
 * Returns a file descriptor >= 0 on success, -1 on error.
 */
static int try_open_int(struct pal_int *pal_int, const char *d_name,
	const char *name, const char *name_data)
{
	int fd;
	int rc;

	/* If this isn't what the user requested, keep going */
	if (strcmp(name, name_data) != 0) {
		errno = ENOENT;
		return -1;
	}

	rc = get_device_name(pal_int, d_name);
	if (rc == -1) {
		errno = ENOENT;
		return -1;
	}

	fd = open(pal_int->devname, O_RDWR);
	print_open_pathname(fd, pal_int->devname);
	if (fd == -1)
		return -1;

	pal_int->fd = fd;
	return fd;
}

/*
 * Try to open the given name.
 * pal_int:	Pointer to the struct pal_int we are working on
 * d_name:	Name of the UIO directory
 * name:	Name the user requested to open
 * name_data:	Contents of the name file in the UIO directory
 *
 * Returns a file descriptor >= 0 on success, -1 on error.
 */
static int try_open_int_msi(struct pal_int *pal_int, const char *d_name,
	const char *name, const char *name_data)
{
	const char *p;
	ptrdiff_t len;
	int rc;
	int fd;
	char *endptr;

	/* Parse off and verify the trailing ".<n>" */
	p = strrchr(name_data, '.');
	if (p == NULL) {
		errno = ENOENT;
		return -1;
	}
	/* Check for a trailing number, which must be base 10 */
	(void)strtoul(p + 1, &endptr, 10);
	if (*(p + 1) == '\0' || *endptr != '\0') {
		errno = ENOENT;
		return -1;
	}

	/*
	 * Now match what the user gave us with the contents of the UIO name
	 * file, less the trailing ".<n>"
	 */
	len = p - name_data;
	if (strncmp(name, name_data, len) != 0) {
		errno = ENOENT;
		return -1;
	}

	/* Compute the name in /dev and open that file */
	rc = get_device_name(pal_int, d_name);
	if (rc == -1) {
		errno = ENOENT;
		return -1;
	}
	fd = open(pal_int->devname, O_RDONLY);
	print_open_pathname(fd, pal_int->devname);
	if (fd == -1)
		return -1;

	pal_int->fd = fd;
	return fd;
}

/*
 * Common open code.
 * pal_int:	Pointer to the struct pal_int we are working on
 * name:	Name of the thing to open
 * try_open:	Pointer to function to try to open a UIO device
 */
static int pal_int_open_common(struct pal_int *pal_int, const char *name,
	int (*try_open)(struct pal_int *pal_int, const char *d_name,
		const char *name, const char *name_data))
{
	DIR *dirp;
	struct dirent *dirent;
	int fd;

	memset(pal_int, 0, sizeof(*pal_int));

	/* Open the directory in sysfs containing all of the UIO directories */
	pal_int->uio_dir_fd = open(SYSFS_UIO_DIR, O_RDONLY);
	print_open_pathname(pal_int->uio_dir_fd, SYSFS_UIO_DIR);
	if (pal_int->uio_dir_fd < 0)
		return -1;

	/* Get ready to iterate over all the UIO directories */
	dirp = opendir(SYSFS_UIO_DIR);
	if (dirp == NULL) {
		close(pal_int->uio_dir_fd);
		pal_int->uio_dir_fd = -1;
		return -1;
	}

	/*
	 * It would be nice to cache the location in the directory read and
	 * start each search from the cached location, but support for this
	 * is not in the directory reading interface. It might be possible
	 * to use lseek() for this but it seems that would be relying on
	 * undocumented behavior.
	 */
	errno = 0;
	for (dirent = readdir(dirp); dirent != NULL; dirent = readdir(dirp)) {
		char name_data[256];
		unsigned long n;
		ssize_t zrc;


		/* Read the name file from this directory, if it's there */
		if (!is_uio_name(dirent->d_name, &n))
			continue;
		pal_int->n = n;
		zrc = read_file_text(pal_int->uio_dir_fd, name_data,
			sizeof(name_data), "%s/%s", dirent->d_name, "name");
		if (zrc < 0)
			continue;

		/* Try to open it */
		fd = try_open(pal_int, dirent->d_name, name, name_data);
		if (fd >= 0)
			break;

		/* Reset errno each time so we can detect the difference
		 * between readdir indicating the end of entries and readdir
		 * returning an error */
		errno = 0;
	}

	if (closedir(dirp) != 0)
		return -1;

	/* We detect errors through use of errno */
	if (errno != 0)
		return -1;
	/* If we got to the end of the list, we failed */
	if (dirent == NULL) {
		errno = ENOENT;
		return -1;
	}

	return fd;
}

/*
 * Open the pengic UIO device
 */
int pal_int_open(struct pal_int *pal_int, const char *name)
{
	int rc;

	if (pal_int_open_common(pal_int, name, try_open_int) == -1)
		return -1;

	rc = get_device_info(name, &pal_int->enable_mask,
		&pal_int->csr_paddr);
	if (rc < 0)
		goto close_fd;
	pal_int->have_int_enable = true;

	return pal_int->fd;

close_fd:
	close(pal_int->fd);
	return rc;
}

int pal_int_open_msi(struct pal_int *pal_int, uint64_t *msgaddr,
	uint32_t *msgdata)
{
	char buf[12];
	ssize_t rc;
	uint32_t lo, hi;
	const char *name = "uio_penmsi1";

	if (pal_int_open_common(pal_int, name, try_open_int_msi) == -1)
		return -1;

	rc = read_file_binary(pal_int->uio_dir_fd, buf, sizeof(buf),
		"uio%u/%s", pal_int->n, "pensando/msi");
	if (rc == -1)
		return -1;
	if (rc != sizeof(buf)) {
		errno = EIO;
		return -1;
	}

	/* This is rather pendantic but avoids possible alignment problems */
	memcpy(&lo, &buf[0], sizeof(*msgaddr) / 2);
	memcpy(&hi, &buf[sizeof(*msgaddr) / 2], sizeof(*msgaddr) / 2);
	*msgaddr = (int64_t)hi << 32 | lo;
	memcpy(msgdata, &buf[sizeof(msgaddr)], sizeof(*msgdata));
	return pal_int->fd;
}

/*
 * Closes files and free resources associated with the given pal_int
 */
int pal_int_close(struct pal_int *pal_int)
{
	unsigned i;
	int rc;

	for (i = 0; i < pal_int->n_mappings; i++) {
		struct mapping_info *mi;

		mi = &pal_int->mapping_info[i];
		munmap(mi->vaddr, mi->devsize);
	}

	rc = close(pal_int->fd);
	pal_int->fd = -1;	/* Block further use of pal_int*() functions */
	return rc;
}

/*
 * This is called to wait for the next interrupt and, when one happens, to
 * return the cummulative number of interrupts seen so far.
 */
int pal_int_start(struct pal_int *pal_int)
{
	uint32_t count;
	ssize_t res;

	res = read(pal_int->fd, &count, sizeof(count));
	if (res < 0) {
		return -1;
	}
	if (res != sizeof(count)) {
		errno = EIO;
		return -1;
	}

	return count;
}

/*
 * This indicates that the processing started by a call to pal_int_start() is
 * done and that the application is finished with the current interrupt.
 */
int pal_int_end(struct pal_int *pal_int)
{
#ifdef EXPLICIT_WRITE_FOR_EOI
	if (pal_int->have_int_enable) {
		ssize_t zrc;
		uint32_t irq_on;

		irq_on = 1;
		zrc = write(pal_int->fd, &irq_on, sizeof(irq_on));
		if (zrc == -1)
			return -1;
		if ((size_t)zrc != sizeof(irq_on)) {
			errno = EIO;
			return -1;
		}
	}
#endif

	return 0;
}

int pal_int_fd(struct pal_int *pal_int)
{
	return pal_int->fd;
}
