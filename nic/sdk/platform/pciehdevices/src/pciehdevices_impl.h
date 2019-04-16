/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#ifndef __PCIEHDEVICES_IMPL_H__
#define __PCIEHDEVICES_IMPL_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef struct pciehdevice_s {
    const char *name;
    int (*init_bars)  (pciehdev_t *pdev, const pciehdev_res_t *res);
    int (*init_cfg)   (pciehdev_t *pdev, const pciehdev_res_t *res);
    int (*init_vfbars)(pciehdev_t *pdev, const pciehdev_res_t *res);
    int (*init_vfcfg) (pciehdev_t *pdev, const pciehdev_res_t *res);
} pciehdevice_t;

void pciehdevice_register(pciehdevice_t *pdevice);

#define PCIEHDEVICE_REGISTER(pd) \
    static void pciehdevice_register_##pd(void) __attribute__((constructor)); \
    static void pciehdevice_register_##pd(void) { pciehdevice_register(&pd); }

void add_msix_region(pciehbars_t *pbars,
                     pciehbar_t *pbar,
                     const pciehdev_res_t *res,
                     const u_int32_t msixtbloff,
                     const u_int32_t msixpbaoff);

void add_common_resource_bar(pciehbars_t *pbars, const pciehdev_res_t *res);
void add_common_doorbell_bar(pciehbars_t *pbars,
                             const pciehdev_res_t *res,
                             const u_int8_t upd[8]);
void add_common_cmb_bar(pciehbars_t *pbars, const pciehdev_res_t *res);
void add_common_rom_bar(pciehbars_t *pbars, const pciehdev_res_t *res);

#endif /* __PCIEHDEVICES_IMPL_H__ */
