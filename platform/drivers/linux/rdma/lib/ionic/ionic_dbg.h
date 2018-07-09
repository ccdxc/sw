#ifndef IONIC_DBG_H
#define IONIC_DBG_H

/* for now, debug is always enabled */
#define IONIC_DEBUG

/* this file may be included from other headers that define these types */
struct ionic_ctx;

#ifdef IONIC_DEBUG
#include <stdio.h>
static inline void ionic_type_ctx(struct ionic_ctx *ctx) {}
#define ionic_dbg(ctx, fmt, args...) do { \
	ionic_type_ctx(ctx); \
	fprintf(stderr, "%s:%d: " fmt "\n", __func__, __LINE__, ##args); \
} while (0)
/* TODO: fprintf(ctx->dbg_fd, "%s:%d" fmt "\n", __func__, __LINE__, ##args); */
#else
static inline void ionic_dbg(struct ionic_ctx *ctx, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
static inline void ionic_dbg(struct ionic_ctx *ctx, const char *fmt, ...) {}
#endif

static inline void ionic_dbg_xdump(struct ionic_ctx *ctx, const char *str,
				   const void *ptr, size_t size)
{
	const uint8_t *ptr8 = ptr;
	int i;

	for (i = 0; i < size; i += 8)
		ionic_dbg(ctx, "%s: %02x %02x %02x %02x %02x %02x %02x %02x",
			  str,
			  ptr8[i + 0], ptr8[i + 1], ptr8[i + 2], ptr8[i + 3],
			  ptr8[i + 4], ptr8[i + 5], ptr8[i + 6], ptr8[i + 7]);
}

#endif
