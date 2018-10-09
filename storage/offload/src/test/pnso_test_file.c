/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

/* TODO: remove user-space specific includes */
#include <stdio.h>
#include <stdlib.h>

#include "osal_stdtypes.h"
#include "osal_errno.h"
#include "osal_mem.h"

/* Linux specific */
#include <sys/stat.h>

#include "pnso_test.h"

uint32_t test_file_size(const char *fname)
{
	struct stat stat_buf;

	if (0 != stat(fname, &stat_buf)) {
		return 0;
	}

	if (!S_ISREG(stat_buf.st_mode)) {
		return 0;
	}

	return (uint32_t) stat_buf.st_size;
}


/*
 * Read a file and store in newly allocated buffer
 * Return NULL on error
 */
uint8_t *test_alloc_and_read_file(const char *fname, uint32_t foffset, uint32_t *len)
{
	FILE *f;
	uint8_t *buf;
	struct stat stat_buf;
	uint32_t max_len = *len;

	*len = 0;

	f = fopen(fname, "r");
	if (f == NULL) {
		return NULL;
	}
	if (foffset) {
		if (0 != fseek(f, foffset, SEEK_SET)) {
			fclose(f);
			return NULL;
		}
	}

	if (0 != fstat(fileno(f), &stat_buf)) {
		fclose(f);
		return NULL;
	}

	if (!S_ISREG(stat_buf.st_mode) || !stat_buf.st_size) {
		fclose(f);
		return NULL;
	}
	if (!max_len || max_len > stat_buf.st_size) {
		max_len = stat_buf.st_size;
	}

	buf = TEST_ALLOC(max_len);
	if (!buf) {
		fclose(f);
		return NULL;
	}

	*len = fread(buf, 1, max_len, f);
	if (!(*len)) {
		TEST_FREE(buf);
		buf = NULL;
	}

	fclose(f);
	return buf;
}


/* Read a file's contents into the given buffer list */
pnso_error_t test_read_file(const char *fname, struct pnso_buffer_list *buflist,
			    uint32_t foffset, uint32_t flen)
{
	FILE *f;
	uint32_t read_len;
	size_t i;
	pnso_error_t err = PNSO_OK;

	f = fopen(fname, "r");
	if (f == NULL) {
		return ENOENT;
	}
	if (foffset) {
		if (0 != fseek(f, foffset, SEEK_SET)) {
			err = ENOENT;
			goto done;
		}
	}

	for (i = 0; i < buflist->count; i++) {
		if (!f  || 0 == buflist->buffers[i].len) {
			buflist->buffers[i].buf = 0;
			buflist->buffers[i].len = 0;
			continue;
		}
		read_len = buflist->buffers[i].len;
		if (flen && read_len > flen)
			read_len = flen;

		buflist->buffers[i].len = fread((void*)buflist->buffers[i].buf,
						1, read_len, f);
		if (!buflist->buffers[i].len) {
			err = ferror(f);
			fclose(f);
			f = NULL;
			/* Don't break, since we still need to clear buflist */
		}
		if (flen) {
			flen -= buflist->buffers[i].len;
		}
	}

done:
	if (f) {
		fclose(f);
	}

	return err;
}

/* Write from the given buffer list into a file */
pnso_error_t test_write_file(const char *fname,
			     const struct pnso_buffer_list *buflist,
			     uint32_t flen, uint32_t flags)
{
	FILE *f;
	size_t i, len, tmp;
	pnso_error_t err  = PNSO_OK;

	if (flags & TEST_OUTPUT_FLAG_APPEND) {
		f = fopen(fname, "a");
	} else {
		f = fopen(fname, "w");
	}
	if (f == NULL) {
		return ENOENT;
	}

	for (i = 0; i < buflist->count && flen > 0; i++) {
		if (!buflist->buffers[i].len) {
			continue;
		}

		tmp = buflist->buffers[i].len;
		if (tmp > flen) {
			tmp = flen;
		}

		len = fwrite((void*)buflist->buffers[i].buf,
			     1, tmp, f);
		if (len != tmp) {
			err = ferror(f);
			break;
		}
		flen -= len;
	}

	fclose(f);
	return err;
}

/* Delete given file from system */
pnso_error_t test_delete_file(const char *fname)
{
	if (remove(fname) != 0) {
		return errno;
	}
	return PNSO_OK;
}

#define TEST_FREAD_SIZE 4096

int test_compare_files(const char *path1, const char *path2,
		       uint32_t foffset, uint32_t flen)
{
	int ret = 0;
	FILE *f1, *f2;
	size_t len1, len2, read_len;
	uint8_t buf1[TEST_FREAD_SIZE];
	uint8_t buf2[TEST_FREAD_SIZE];

	f1 = fopen(path1, "r");
	if (f1 == NULL) {
		return -2;
	}
	f2 = fopen(path2, "r");
	if (f2 == NULL) {
		fclose(f1);
		return -2;
	}

	if (foffset) {
		if (0 != fseek(f1, foffset, SEEK_SET) ||
		    0 != fseek(f2, foffset, SEEK_SET)) {
			ret = -2;
			goto done;
		}
	}

	while (ret == 0) {
		read_len = TEST_FREAD_SIZE;
		if (flen && read_len > flen) {
			read_len = flen;
		}
		len1 = fread(buf1, 1, read_len, f1);
		len2 = fread(buf2, 1, read_len, f2);
		if (len1 < len2) {
			ret = -1;
		} else if (len1 > len2) {
			ret = 1;
		} else if (len1) {
			ret = memcmp(buf1, buf2, len1);
		} else {
			/* hit EOF */
			break;
		}
		if (flen) {
			/* Exit early in case max length specified */
			flen -= len1;
			if (!flen) {
				break;
			}
		}
	}

done:
	fclose(f1);
	fclose(f2);
	return ret;
}

int test_compare_file_data(const char *path,
			   uint32_t foffset, uint32_t flen,
			   const uint8_t *pat, uint32_t pat_len)
{
	int ret = 0;
	FILE *f;
	uint32_t i, pat_i = 0;
	size_t len, read_len;
	uint8_t buf[TEST_FREAD_SIZE];

	f = fopen(path, "r");
	if (f == NULL) {
		return -2;
	}

	if (foffset) {
		if (0 != fseek(f, foffset, SEEK_SET)) {
			ret = -2;
			goto done;
		}
	}

	while (ret == 0) {
		read_len = TEST_FREAD_SIZE;
		if (flen && read_len > flen) {
			read_len = flen;
		}
		len = fread(buf, 1, read_len, f);
		if (len) {
			for (i = 0; i < len; i++) {
				ret = (int) buf[i] - (int) pat[pat_i++ % pat_len];
				if (ret)
					goto done;
			}
		} else {
			/* hit EOF */
			break;
		}
		if (flen) {
			/* Exit early in case max length specified */
			flen -= len;
			if (!flen) {
				break;
			}
		}
	}

done:
	fclose(f);
	return ret;
}
