#ifndef IONIC_DBG_H
#define IONIC_DBG_H

/* for now, debug is always enabled */
#define IONIC_DEBUG
//#define IONIC_VERBOSE_DEBUG

/* this file may be included from other headers that define these types */
struct ionic_ctx;
struct cqwqe_be_t;
/* mark used and type check the parameters when debug code is not generated */
static inline void ionic_type_ctx(struct ionic_ctx *ctx) {}
static inline void ionic_type_cqe(struct cqwqe_be_t *cqe) {}

#ifdef IONIC_DEBUG
#include <stdio.h>
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

#define ionic_dbg_cqe_verbose(ctx, cqe) do { \
	ionic_type_cqe(cqe); \
	ionic_dbg(ctx, #cqe "->id.msn\t= %#x", (cqe)->id.msn); \
	ionic_dbg(ctx, #cqe "->op_type\t= %#x", (cqe)->op_type); \
	ionic_dbg(ctx, #cqe "->status\t= %#x", (cqe)->status); \
	ionic_dbg(ctx, #cqe "->qp_hi\t= %#x", (cqe)->qp_hi); \
	ionic_dbg(ctx, #cqe "->qp_lo\t= %#x", (cqe)->qp_lo); \
	ionic_dbg(ctx, #cqe "->src_qp_hi\t= %#x", (cqe)->src_qp_hi); \
	ionic_dbg(ctx, #cqe "->src_qp_lo\t= %#x", (cqe)->src_qp_lo); \
	ionic_dbg(ctx, #cqe "->smac[0]\t= %#x", (cqe)->smac[0]); \
	ionic_dbg(ctx, #cqe "->smac[1]\t= %#x", (cqe)->smac[1]); \
	ionic_dbg(ctx, #cqe "->smac[2]\t= %#x", (cqe)->smac[2]); \
	ionic_dbg(ctx, #cqe "->color_flags\t= %#x", (cqe)->color_flags); \
	ionic_dbg(ctx, #cqe "->imm_data\t= %#x", (cqe)->imm_data); \
	ionic_dbg(ctx, #cqe "->r_key\t= %#x", (cqe)->r_key); \
} while(0)

#define ionic_dbg_cqe_quiet(ctx, cqe) do { \
	ionic_type_cqe(cqe); \
	ionic_dbg(ctx, "id.msn=%#x op_type=%#x status=%#x color_flags=%#x", \
		  (cqe)->id.msn, (cqe)->op_type, (cqe)->status, \
		  (cqe)->color_flags); \
} while(0)

#ifdef IONIC_VERBOSE_DEBUG
#define ionic_dbg_cqe ionic_dbg_cqe_verbose
#else
#define ionic_dbg_cqe ionic_dbg_cqe_quiet
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

/* XXX deprecate: replace use of IONIC_LOG in the code with ionic_dbg */
#define IONIC_LOG(args...) ionic_dbg(NULL, ##args)

#endif
