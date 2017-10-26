/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHCFG_H__
#define __PCIEHCFG_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct pciehcfg_s;
typedef struct pciehcfg_s pciehcfg_t;
struct pciehbars_s;
typedef struct pciehbars_s pciehbars_t;
struct cfgspace_s;
typedef struct cfgspace_s cfgspace_t;

pciehcfg_t *pciehcfg_new(void);
void pciehcfg_delete(pciehcfg_t *pcfg);
void pciehcfg_get_cfgspace(pciehcfg_t *pcfg, cfgspace_t *cs);
void pciehcfg_set_bars(pciehcfg_t *pcfg, pciehbars_t *pbars);

/*
 * Configuration parameters.
 */
void pciehcfg_setconf_cap_gen(pciehcfg_t *pcfg, const u_int8_t cap_gen);
void pciehcfg_setconf_cap_width(pciehcfg_t *pcfg, const u_int8_t cap_width);
void pciehcfg_setconf_vendorid(pciehcfg_t *pcfg, const u_int16_t vendorid);
void pciehcfg_setconf_deviceid(pciehcfg_t *pcfg, const u_int16_t deviceid);
void pciehcfg_setconf_subvendorid(pciehcfg_t *pcfg, const u_int16_t subvid);
void pciehcfg_setconf_subdeviceid(pciehcfg_t *pcfg, const u_int16_t subdid);
void pciehcfg_setconf_classcode(pciehcfg_t *pcfg, const u_int32_t classcode);
void pciehcfg_setconf_revid(pciehcfg_t *pcfg, const u_int8_t revid);
void pciehcfg_setconf_intpin(pciehcfg_t *pcfg, const u_int8_t intpin);
void pciehcfg_setconf_nintrs(pciehcfg_t *pcfg, const u_int16_t nintrs);
void pciehcfg_setconf_msix_tblbir(pciehcfg_t *pcfg, const u_int8_t bir);
void pciehcfg_setconf_msix_tbloff(pciehcfg_t *pcfg, const u_int32_t off);
void pciehcfg_setconf_msix_pbabir(pciehcfg_t *pcfg, const u_int8_t bir);
void pciehcfg_setconf_msix_pbaoff(pciehcfg_t *pcfg, const u_int32_t off);
void pciehcfg_setconf_dsn(pciehcfg_t *pcfg, const u_int64_t dsn);
void pciehcfg_setconf_flr(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_exttag(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_exttag_en(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_msicap(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_msixcap(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_bridgeup(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_bridgedn(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_fnn(pciehcfg_t *pcfg, const int on);
void pciehcfg_setconf_vf(pciehcfg_t *pcfg, const int on);

/******************************************************************/

/*
 * Common header registers.
 */
void pciehcfg_sethdr_type0(pciehcfg_t *pcfg, pciehbars_t *pbars);
void pciehcfg_sethdr_type1(pciehcfg_t *pcfg, pciehbars_t *pbars);

/*
 * Capabilities management.
 */
void pciehcfg_addcap(pciehcfg_t *pcfg, const char *capname);
void pciehcfg_addextcap(pciehcfg_t *pcfg, const char *capname);
void pciehcfg_add_standard_caps(pciehcfg_t *pcfg);

void pciehcfg_make_fn0(pciehcfg_t *pcfg);
void pciehcfg_make_fnn(pciehcfg_t *pcfg, const int fnn);

/******************************************************************/

/*
 * Config space operational accessors.
 *
 * Reads return current values, writes apply the write-mask to
 * implement read-only fields.
 */
int pciehcfg_generic_read(pciehcfg_t *pcfg,
                          const u_int16_t offset,
                          const u_int8_t size,
                          u_int32_t *valp);

int pciehcfg_generic_write(pciehcfg_t *pcfg,
                           const u_int16_t offset,
                           const u_int8_t size,
                           const u_int32_t val);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHCFG_H__ */
