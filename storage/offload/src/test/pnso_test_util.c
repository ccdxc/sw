/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <string.h>
#include <ctype.h>
#include "osal_stdtypes.h"
#include "pnso_api.h"

#ifdef __KERNEL__
#include <linux/random.h>
#define pnso_srand prandom_seed
#define pnso_rand prandom_u32
#else
#include <stdlib.h>
#define pnso_srand srand
#define pnso_rand rand
#endif


/* Fill buflist with random values */
pnso_error_t test_fill_random(struct pnso_buffer_list *buflist, uint32_t seed)
{
	size_t i, j;
	uint32_t rnum = 0;
	uint8_t *dst;

	pnso_srand(seed);

	for (i = 0; i < buflist->count; i++) {
		if (!buflist->buffers[i].len) {
			continue;
		}


		dst = (uint8_t *) buflist->buffers[i].buf;
		for (j = 0; j < buflist->buffers[i].len; j++) {
			if ((j % 4) == 0) {
				rnum = pnso_rand();
			}
			dst[j] = (rnum >> ((j % 4) * 8)) & 0xff;
		}
	}

	return PNSO_OK;
}

/* Convert binary to decimal ASCII, returning length */
uint32_t safe_itoa(char *dst, uint32_t dst_len, uint64_t val)
{
	uint32_t ret, len = 0;
	uint64_t tmp;

	if (!dst || !dst_len) {
		return 0;
	}
	if (val == 0) {
		dst[0] = '0';
		dst[1] = '\0';
		return 1;
	}

	/* First calculate length */
	tmp = val;
	while (tmp) {
		len++;
		tmp /= 10;
	}

	if (len >= dst_len) {
		*dst = '\0';
		return 0;
	}

	/* Convert to string */
	ret = len;
	tmp = val;
	dst[len] = '\0';
	while (len) {
		dst[--len] = '0' + (tmp % 10);
		tmp /= 10;
	}

	return ret;
}

uint32_t safe_strcpy_tolower(char *dst, const char *src, uint32_t max_len)
{
	uint32_t len;

	if (dst == NULL)
		return 0;

	if (src == NULL) {
		*dst = '\0';
		return 0;
	}

	for (len = 0; src[len] != '\0' && len < (max_len -1); len++) {
		dst[len] = tolower(src[len]);
	}
	dst[len] = '\0';

	return len;
}

uint32_t safe_strcpy(char *dst, const char *src, uint32_t max_len)
{
	uint32_t len;

	if (dst == NULL)
		return 0;

	if (src == NULL) {
		*dst = '\0';
		return 0;
	}

	for (len = 0; src[len] != '\0' && len < (max_len -1); len++) {
		dst[len] = src[len];
	}
	dst[len] = '\0';

	return len;
}

/* String compare which allows NULL inputs */
/* Treat empty string and NULL string as equivalent */
int safe_strcmp(const char *str1, const char *str2)
{
        char empty_str[1] = "";

	if (!str1) {
		str1 = empty_str;
	}
	if (!str2) {
		str2 = empty_str;
	}

	return strcmp(str1, str2);
}

static inline unsigned int char2val(char c)
{
	if (c >= '0' && c <= '9')
		return (unsigned int) c - '0';
	if (c >= 'a' && c <= 'z')
		return (unsigned int) (10 + c - 'a');
	if (c >= 'A' && c <= 'Z')
		return (unsigned int) (10 + c - 'A');
	return 0;
}

static unsigned long long str_to_dec(const char *val)
{
	unsigned long long ret = 0;

	while (*val) {
		if (isdigit(*val))
			ret = 10*ret + char2val(*val);
		else
			break;
		val++;
	}
	return ret;
}

static unsigned long long str_to_hex(const char *val)
{
	unsigned long long ret = 0;

	while (*val) {
		if (isxdigit(*val))
			ret = 16*ret + char2val(*val);
		else
			break;
		val++;
	}
	return ret;
}
	 
unsigned long long safe_strtoll(const char *val)
{
	if (!val || !isdigit(*val)) {
		return -1;
	}

	if (val[0] == '0' &&
	    (val[1] == 'x' || val[1] == 'X')) {
		return str_to_hex(val+2);
	}

	return str_to_dec(val);
}
