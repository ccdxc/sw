/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEHDEVICES_IMPL_H__
#define __PCIEHDEVICES_IMPL_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

void
add_common_resource_bar(pciehbars_t *pbars,
                        const pciehdevice_resources_t *pres);
void
add_common_doorbell_bar(pciehbars_t *pbars,
                        const pciehdevice_resources_t *pres,
                        const u_int8_t upd[8]);
void
add_common_cmb_bar(pciehbars_t *pbars,
                   const pciehdevice_resources_t *pres);

void
add_common_rom_bar(pciehbars_t *pbars,
                   const pciehdevice_resources_t *pres);

#endif /* __PCIEHDEVICES_IMPL_H__ */
