/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __INTRUTILS_H__
#define __INTRUTILS_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef struct intr_drvcfg_s {
    u_int32_t coal_init;
    u_int32_t mask;
    u_int32_t int_credits;
    u_int32_t mask_on_assert;
    u_int32_t coal_curr;
} __attribute__((packed)) intr_drvcfg_t;

typedef struct intr_msixcfg_s {
    u_int64_t msgaddr;
    u_int32_t msgdata;
    u_int32_t vector_ctrl;
} __attribute__((packed)) intr_msixcfg_t;

typedef union intr_fwcfg_u {
    struct {
        u_int32_t function_mask:1;
        u_int32_t rsrv:31;
        u_int32_t lif:11;
        u_int32_t port_id:3;
        u_int32_t local_int:1;
        u_int32_t legacy:1;
        u_int32_t int_pin:2;
        u_int32_t rsrv2:14;
    } __attribute__((packed));
    u_int32_t w[2];
} intr_fwcfg_t;

typedef union intr_state_s {
    struct {
        u_int64_t msixcfg_msg_addr_51_2:50;
        u_int64_t msixcfg_msg_data_lo:14;
        u_int64_t msixcfg_msg_data_hi:18;
        u_int64_t msixcfg_vector_ctrl:1;
        u_int64_t fwcfg_function_mask:1;
        u_int64_t fwcfg_lif:11;
        u_int64_t fwcfg_local_int:1;
        u_int64_t fwcfg_legacy_int:1;
        u_int64_t fwcfg_legacy_pin:2;
        u_int64_t drvcfg_mask:1;
        u_int64_t drvcfg_int_credits:16;
        u_int64_t drvcfg_mask_on_assert:1;
        u_int64_t fwcfg_port_id:3;
        u_int64_t rsvd;
    };
    u_int32_t w[4];
} __attribute__((packed)) intr_state_t;

u_int64_t intr_msixcfg_addr(const int intr);
u_int64_t intr_fwcfg_addr(const int intr);
u_int64_t intr_drvcfg_addr(const int intr);
u_int64_t intr_assert_addr(const int intr);
u_int64_t intr_msixcfg_addr(const int intrb);
u_int32_t intr_msixcfg_size(const int intrc);
u_int64_t intr_fwcfg_addr(const int intrb);
u_int64_t intr_drvcfg_addr(const int intrb);
u_int32_t intr_drvcfg_size(const int intrc);
u_int64_t intr_assert_addr(const int intrb);
u_int64_t intr_pba_addr(const int lif);
u_int32_t intr_pba_size(const int intrc);
u_int64_t intr_pba_cfg_addr(const int lif);
u_int64_t intr_state_addr(const int intr);

void intr_drvcfg(const int intr);
void intr_pba_cfg(const int lif, const int intrb, const size_t intrc);
void intr_pba_clear(const int intr);
void intr_msixcfg(const int intr,
                  const u_int64_t msgaddr,
                  const u_int32_t msgdata,
                  const int vctrl);
void intr_fwcfg(const int intr,
                const int lif,
                const int port,
                const int legacy,
                const int intpin,
                const int fmask);
void intr_fwcfg_msi(const int intr, const int lif, const int port);
void intr_fwcfg_legacy(const int intr,
                       const int lif, const int port, const int intpin);
int intr_fwcfg_function_mask(const int intr, const int on);
void intr_fwcfg_mode(const int intr, const int legacy, const int fmask);
void intr_fwcfg_local(const int intr, const int on);
void intr_state(const int intr, intr_state_t *v);

void intr_hwinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __INTRUTILS_H__ */
