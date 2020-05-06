/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
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
        u_int64_t msixcfg_msg_data:32;
        u_int64_t msixcfg_vector_ctrl:1;
        u_int64_t fwcfg_function_mask:1;
        u_int64_t fwcfg_lif:11;
        u_int64_t fwcfg_local_int:1;
        u_int64_t fwcfg_legacy_int:1;
        u_int64_t fwcfg_legacy_pin:2;
        u_int64_t drvcfg_mask:1;
          int64_t drvcfg_int_credits:16; /* signed */
        u_int64_t drvcfg_mask_on_assert:1;
        u_int64_t fwcfg_port_id:3;
    } __attribute__((packed));
    u_int32_t w[4];
} intr_state_t;

/*
 * Simple resource address accessors.
 */
u_int64_t intr_msixcfg_addr(const int intrb);
u_int32_t intr_msixcfg_size(const int intrc);
u_int64_t intr_fwcfg_addr(const int intrb);
u_int64_t intr_drvcfg_addr(const int intrb);
u_int32_t intr_drvcfg_size(const int intrc);
u_int64_t intr_assert_addr(const int intrb);
u_int32_t intr_assert_stride(void);
u_int32_t intr_assert_data(void);
u_int64_t intr_pba_addr(const int lif);
u_int32_t intr_pba_size(const int intrc);
u_int64_t intr_pba_cfg_addr(const int lif);

/*
 * Interrupt resource configuration/initialization.
 */
void
intr_drvcfg(const int intr,
            const int mask, const int coal_init, const int mask_on_assert);
int intr_drvcfg_mask(const int intr, const int on);
void intr_pba_cfg(const int lif, const int intrb, const size_t intrc);
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
int intr_config_local_msi(const int intr,
                          u_int64_t msgaddr, u_int32_t msgdata);

/* Interrupt state accessor. */
void intr_state_get(const int intr, intr_state_t *st);
void intr_state_set(const int intr, intr_state_t *st);

/* interrupt coalescing */
void intr_coal_set_resolution(const int ticks);
int intr_coal_get_resolution(void);
int intr_coal_get_params(int *mul, int *div);

/*
 * Reset functions.  These functions have a "dmask" parameter for the
 *     reset value of the drvcfg.mask register.  If the device has
 *     interrupt resources mapped to the host and the interrupt
 *     resource is managed by the host driver then call with dmask=1
 *     so the interrupt is masked at reset.  The driver will unmask
 *     it through its bar when initialization is complete.  If the
 *     device is using interrupt resources but the host driver does
 *     *not* directly manage the interrupt resource then set dmask=0
 *     so the interrupts reset to "unmasked" and can be fired when
 *     the local fw is ready to send.
 *
 * intr_reset_pci() - reset the pcie managed register groups to default values,
 *                    use for pcie block resets (FLR, bus reset).
 * intr_reset_dev() - device requested interrupt reset, reset all driver
 *                    owned registers.
 */
void intr_reset_pci(const int intrb, const int intrc, const int dmask);
void intr_reset_dev(const int intrb, const int intrc, const int dmask);

void intr_reset_pba(const int intrb, const int intrc);

void intr_hwinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __INTRUTILS_H__ */
