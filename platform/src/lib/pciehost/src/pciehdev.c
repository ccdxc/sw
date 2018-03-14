/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include "misc.h"
#include "bdf.h"
#include "pci_ids.h"
#include "pciehdevices.h"
#include "pciehdev.h"
#include "pciehdev_impl.h"
#include "pciehcfg.h"
#include "pciehbar.h"
#include "pciehsys.h"
#include "pciehw.h"

#define NPORTS 8

typedef struct pciehdev_portinfo_s {
    u_int32_t finalized:1;
    u_int32_t memtun_br:1;
    pciehdev_t *root;
} pciehdev_portinfo_t;

typedef struct pciehdev_data_s {
    pciehdev_openparams_t params;
    pciehdev_evhandler_t evhandler;
    pciehdev_portinfo_t portinfo[NPORTS];
} pciehdev_data_t;

static pciehdev_data_t pciehdev_data;

static pciehdev_data_t *
pciehdev_get_data(void)
{
    return &pciehdev_data;
}

static int
pciehdev_port_is_enabled(const u_int8_t port)
{
    pciehdev_openparams_t *params = pciehdev_get_params();
    return (params->enabled_ports & (1 << port)) != 0;
}

/******************************************************************/

int
pciehdev_open(pciehdev_openparams_t *params)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehw_params_t hwparams;
    int r;

    if (params) {
        pdevdata->params = *params;
    }

    if (pdevdata->params.vendorid == 0) {
        /* vendorid defaults to Pensando. */
        pdevdata->params.vendorid = PCI_VENDOR_ID_PENSANDO;
    }
    if (pdevdata->params.subvendorid == 0) {
        /* subvendorid defaults to vendorid. */
        pdevdata->params.subvendorid = pdevdata->params.vendorid;
    }
    if (pdevdata->params.cap_gen == 0) {
        pdevdata->params.cap_gen = 4;
    }
    if (pdevdata->params.cap_width == 0) {
        pdevdata->params.cap_width = 16;
    }
    if (pdevdata->params.enabled_ports == 0) {
        /* port 0 enabled by default */
        pdevdata->params.enabled_ports = 0x1;
    }

    memset(&hwparams, 0, sizeof(hwparams));
    hwparams.inithw = pdevdata->params.inithw;
    hwparams.fake_bios_scan = pdevdata->params.fake_bios_scan;
    hwparams.first_bus = pdevdata->params.first_bus;
    hwparams.enabled_ports = pdevdata->params.enabled_ports;
    if ((r = pciehw_open(&hwparams)) < 0) {
        return r;
    }
    return 0;
}

void
pciehdev_close(void)
{
    pciehw_close();
}

/******************************************************************/

static void
pciehdev_setconf_defaults(pciehdev_t *pdev)
{
}

pciehdev_t *
pciehdev_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehsys_zalloc(sizeof(pciehdev_t));
    assert(pdev != NULL);

    pciehdev_setconf_defaults(pdev);
    if (pres) {
        if (pres->lif_valid) {
            pdev->lif = pres->lif;
            pdev->lif_valid = pres->lif_valid;
        }
        pdev->intrb = pres->intrb;
        pdev->intrc = pres->intrc;
        pdev->port = pres->port;
    }
    strncpy0(pdev->name, name, sizeof(pdev->name));
    return pdev;
}

void
pciehdev_delete(pciehdev_t *pdev)
{
    if (pdev == NULL) return;
    pciehbars_delete(pdev->pbars);
    pciehcfg_delete(pdev->pcfg);
    pciehsys_free(pdev);
}

static void
pciehdev_delete_topology(pciehdev_t *pdev)
{
    if (pdev == NULL) return;
    pciehdev_delete_topology(pdev->child);
    pciehdev_delete_topology(pdev->peer);
    pciehdev_delete(pdev);
}

int
pciehdev_initialize(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    int port;

    for (port = 0; port < NPORTS; port++) {
        pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];

        if (pciehdev_port_is_enabled(port)) {
            if (pi->finalized) {
                pi->finalized = 0;
            }
            pciehw_initialize_topology(port);
            pciehdev_delete_topology(pi->root);
            pi->root = NULL;
            pi->memtun_br = 0;
        }
    }
    return 0;
}

static int
pciehdev_finalize_dev(pciehdev_t *pdev, u_int16_t bdf, u_int8_t *nextbus)
{
    if (pdev) {
        pdev->bdf = bdf;

#if 0
        printf("finalize_dev: %-8s bdf %s par %s peer %s child %s\n",
               pciehdev_get_name(pdev), bdf_to_str(pdev->bdf),
               pdev->parent ? pciehdev_get_name(pdev->parent) : "(none)",
               pdev->peer ? pciehdev_get_name(pdev->peer) : "(none)",
               pdev->child ? pciehdev_get_name(pdev->child) : "(none)");
#endif

        /*
         * Process child link first, then peer, so we match
         * the path of the BIOS scan.  Our assigned bdf's
         * will match the actual bdf's when the BIOS assigns
         * bus numbers to bridges.
         */
        if (pdev->child) {
            int childbdf;
            if (pdev->fn0) {
                childbdf = bdf_make(bdf_to_bus(bdf),
                                    bdf_to_dev(bdf),
                                    1);
            } else {
                childbdf = bdf_make((*nextbus)++, 0, 0);
            }
            pciehdev_finalize_dev(pdev->child, childbdf, nextbus);
        }

        if (pdev->peer) {
            int peerbdf;
            if (pdev->peer->fnn) {
                peerbdf = bdf_make(bdf_to_bus(bdf),
                                   bdf_to_dev(bdf),
                                   bdf_to_fnc(bdf) + 1);
            } else {
                peerbdf = bdf_make(bdf_to_bus(bdf), bdf_to_dev(bdf) + 1, 0);
            }
            pciehdev_finalize_dev(pdev->peer, peerbdf, nextbus);
        }
    }
    return 0;
}

int
pciehdev_finalize(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    int port;

    for (port = 0; port < NPORTS; port++) {
        pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
        u_int8_t nextbus = pdevdata->params.first_bus;

        if (pciehdev_port_is_enabled(port) && !pi->finalized) {

            /* add a bridge to complete this topology */
            if (pi->root == NULL) {
                pciehdev_t *pbrdn;
                const int memtun_en = port == 0 && pi->memtun_br == 0;
                pi->root = pciehdev_bridgeup_new();
                pi->root->port = port;
                pbrdn = pciehdev_bridgedn_new(memtun_en);
                pbrdn->port = port;
                pciehdev_addchild(pi->root, pbrdn);
                pi->memtun_br = memtun_en;
            }

            pciehdev_finalize_dev(pi->root, bdf_make(0, 0, 0), &nextbus);
            pciehw_finalize_topology(pi->root);
            pi->finalized = 1;
        }
    }
    return 0;
}

void
pciehdev_set_cfg(pciehdev_t *pdev, pciehcfg_t *pcfg)
{
    pdev->pcfg = pcfg;
}

pciehcfg_t *
pciehdev_get_cfg(pciehdev_t *pdev)
{
    return pdev->pcfg;
}

void
pciehdev_set_bars(pciehdev_t *pdev, pciehbars_t *pbars)
{
    pciehbars_finalize(pbars);
    pdev->pbars = pbars;
}

pciehbars_t *
pciehdev_get_bars(pciehdev_t *pdev)
{
    return pdev->pbars;
}

int
pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    const u_int8_t port = pdev->port;
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    pciehdev_t **ppdev;

    if (pi->finalized) {
        return -EBUSY;
    }
    ppdev = pdev ? &pdev->child : &pi->root;
    for (; *ppdev; ppdev = &(*ppdev)->peer) {
        continue;
    }
    *ppdev = pchild;
    if (pchild) {
        pchild->parent = pdev;
    }
    return 0;
}

int
pciehdev_add(pciehdev_t *pdev)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    const u_int8_t port = pdev->port;
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    pciehdev_t *pbrdn;
    int memtun_en = 0;

    if (pdev->port >= NPORTS) {
        return -EINVAL;
    }
    if (pi->finalized) {
        return -EBUSY;
    }
    if (pi->root == NULL) {
        pi->root = pciehdev_bridgeup_new();
        pi->root->port = port;
    }
    memtun_en = port == 0 && pi->memtun_br == 0;
    pbrdn = pciehdev_bridgedn_new(memtun_en);
    pbrdn->port = port;
    pciehdev_addchild(pi->root, pbrdn);
    pi->memtun_br = memtun_en;
    return pciehdev_addchild(pbrdn, pdev);
}

int
pciehdev_addfn(pciehdev_t *pdev, pciehdev_t *pfn, const int fnc)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    const u_int8_t port = pdev->port;
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];

    if (pi->finalized) {
        return -EBUSY;
    }
    pciehdev_make_fn0(pdev);
    pciehdev_make_fnn(pfn, fnc);
    return pciehdev_addchild(pdev, pfn);
}

int
pciehdev_make_fn0(pciehdev_t *pdev)
{
    if (!pdev->fn0) {
        pdev->fn0 = 1;
        pciehcfg_make_fn0(pdev->pcfg);
    }
    return 0;
}

int
pciehdev_make_fnn(pciehdev_t *pdev, const int fnn)
{
    pdev->fnn = fnn;
    pciehcfg_make_fnn(pdev->pcfg, fnn);
    return 0;
}

pciehdev_t *
pciehdev_get_root(const u_int8_t port)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    return pi->root;
}

pciehdev_t *
pciehdev_get_parent(pciehdev_t *pdev)
{
    return pdev->parent;
}

pciehdev_t *
pciehdev_get_child(pciehdev_t *pdev)
{
    return pdev->child;
}

pciehdev_t *
pciehdev_get_peer(pciehdev_t *pdev)
{
    return pdev->peer;
}

static pciehdev_t *
pciehdev_find_by_name(pciehdev_t *pdev, const char *name)
{
    pciehdev_t *pdev_found = NULL;

    if (pdev == NULL) {
        return NULL;
    }
    if (strcmp(pdev->name, name) == 0) {
        pdev_found = pdev;
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_name(pdev->peer, name);
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_name(pdev->child, name);
    }
    return pdev_found;
}

pciehdev_t *
pciehdev_get_by_name(const char *name)
{
    pciehdev_t *pdev_found = NULL;    
    int port;

    for (port = 0; port < NPORTS; port++) {
        pciehdev_t *root = pciehdev_get_root(port);

        pdev_found = pciehdev_find_by_name(root, name);
        if (pdev_found) break;
    }
    return pdev_found;
}

static pciehdev_t *
pciehdev_find_by_bdf(pciehdev_t *pdev, const u_int16_t bdf)
{
    pciehdev_t *pdev_found = NULL;

    if (pdev == NULL) {
        return NULL;
    }
    if (pdev->bdf == bdf) {
        pdev_found = pdev;
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_bdf(pdev->peer, bdf);
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_bdf(pdev->child, bdf);
    }
    return pdev_found;
}

pciehdev_t *
pciehdev_get_by_bdf(const u_int8_t port, const u_int16_t bdf)
{
    pciehdev_t *root = pciehdev_get_root(port);
    return pciehdev_find_by_bdf(root, bdf);
}

void *
pciehdev_get_hwdev(pciehdev_t *pdev)
{
    return pdev->phwdev;
}

void
pciehdev_set_hwdev(pciehdev_t *pdev, void *phwdev)
{
    pdev->phwdev = phwdev;
}

u_int16_t
pciehdev_get_bdf(pciehdev_t *pdev)
{
    return pdev->bdf;
}

u_int8_t
pciehdev_get_port(pciehdev_t *pdev)
{
    return pdev->port;
}

int
pciehdev_get_lif(pciehdev_t *pdev)
{
    return pdev->lif_valid ? pdev->lif : -1;
}

u_int32_t
pciehdev_get_intrb(pciehdev_t *pdev)
{
    return pdev->intrb;
}

u_int32_t
pciehdev_get_intrc(pciehdev_t *pdev)
{
    return pdev->intrc;
}

char *
pciehdev_get_name(pciehdev_t *pdev)
{
    return pdev->name;
}

pciehdev_openparams_t *
pciehdev_get_params(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    return &pdevdata->params;
}

int
pciehdev_register_event_handler(pciehdev_evhandler_t evhandler)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pdevdata->evhandler = evhandler;
    return 0;
}

void
pciehdev_event(pciehdev_t *pdev, const pciehdev_eventdata_t *evd)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();

    if (pdevdata->evhandler) {
        pdevdata->evhandler(pdev, evd);
    }
}
