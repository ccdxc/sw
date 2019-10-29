/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "platform/misc/include/misc.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgrutils/include/pciehcfg_impl.h"
#include "platform/pciemgr/include/pciehw.h"
#include "pciehw_impl.h"

/*
 * This module implements the PCI VPD capability encoding.
 * See PCIe Base Spec, Rev 4.0v1.0 6.28 and 7.9.19 for details.
 */

static void *
pciehw_vpd_getdata(pciehwdev_t *phwdev)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    return pshmem->vpddata[hwdevh];
}

/*
 * Encode a "small" vpd resource type tag.
 */
static size_t
vpd_add_res_s(uint8_t *vpddata,
              const uint8_t id, const uint8_t *buf, const uint8_t buflen)
{
    const uint8_t len = buflen & 0x7;

    if (vpddata) {
        vpddata[0] = (((id & 0xf) << 3) | len);
        memcpy(&vpddata[1], buf, len);
    }
    return len + 1;
}

/*
 * Encode a "large" vpd resource type tag.
 */
static size_t
vpd_add_res_l(uint8_t *vpddata,
              const uint8_t id, const void *buf, const uint16_t buflen)
{
    if (vpddata) {
        vpddata[0] = (0x1 << 7) | (id & 0x7f);
        vpddata[1] = buflen;
        vpddata[2] = buflen >> 8;
        memcpy(&vpddata[3], buf, buflen);
    }
    return buflen + 3;
}

static size_t
vpd_add_key(uint8_t *vpddata,
            const char *key, const void *buf, const uint8_t buflen)
{
    if (vpddata) {
        vpddata[0] = key[0];
        vpddata[1] = key[1];
        vpddata[2] = buflen;
        memcpy(&vpddata[3], buf, buflen);
    }
    return (size_t)buflen + 3;
}

static size_t
vpd_add_key_str(uint8_t *vpddata, const char *key, const char *str)
{
    const uint8_t len = MIN(strlen(str), 0xff);
    return vpd_add_key(vpddata, key, str, len);
}

static size_t
vpd_add_id(uint8_t *vpddata, const char *id)
{
    return vpd_add_res_l(vpddata, 0x02, id, strlen(id));
}

static size_t
vpd_add_ro(uint8_t *vpddata, const uint8_t *buf, const size_t buflen)
{
    return vpd_add_res_l(vpddata, 0x10, buf, buflen);
}

static size_t
vpd_add_csum(uint8_t *vpddata, const uint8_t *buf)
{
    const uint8_t *cp, *eob;
    uint8_t csum;
    size_t sz;

    csum = 0;
    sz = vpd_add_key(vpddata, "RV", &csum, 1);
    if (vpddata) {
        eob = vpddata + sz;
        for (cp = buf; cp < eob; cp++) {
            csum += *cp;
        }
        vpddata[3] = -csum;
    }
    return sz;
}

static size_t
vpd_add_eol(uint8_t *vpddata)
{
    return vpd_add_res_s(vpddata, 0x0f, NULL, 0);
}

/*
 * Encode the info in vpdtab into the vpddata buffer.
 *
 * If vpddata == NULL then calculate the sizes as if we were
 * doing the encode but don't actually copy data.  That way we
 * can reuse the encoding code to determine the size required
 * to validate data fits before copying into the destination buffer.
 */
static size_t
vpdtab_to_vpddata(const vpd_table_t *vpdtab, uint8_t *vpddata)
{
    uint8_t *v = vpddata;
    vpd_entry_t *vpde;
    size_t total = 0;
    size_t n = 0;
    int i;

    if (vpdtab->id) {
        n = vpd_add_id(v, vpdtab->id);
        total += n;
        if (v) v += n;
    }
    if (vpdtab->nentries) {
        uint8_t *vpdro, *vpdro_data;
        size_t rolen;

        vpdro = v;
        n = vpd_add_ro(vpdro, NULL, 0);
        total += n;
        if (v) v += n;
        vpdro_data = v;
        for (vpde = vpdtab->entry, i = 0; i < vpdtab->nentries; i++, vpde++) {
            if (vpde->key && vpde->val) {
                n = vpd_add_key_str(v, vpde->key, vpde->val);
                total += n;
                if (v) v += n;
            }
        }
        if (vpdro) {
            /* read-only length, +4 for csum (below) */
            rolen = (v + 4) - vpdro_data;
            /* update read-only tag length (before csum below) */
            vpdro[1] = rolen;
            vpdro[2] = rolen >> 8;
        }
        /* now calculate csum byte */
        n = vpd_add_csum(v, vpddata);
        total += n;
        if (v) v += n;
    }
    if (vpdtab->id || vpdtab->nentries) {
        n = vpd_add_eol(v);
        total += n;
        if (v) v += n;
    }

    return total;
}

/*
 * Compile the information in vpdtab into the format accessed
 * by the PCI-SIG VPD capability.
 */
void
pciehw_vpd_finalize(pciehwdev_t *phwdev, const vpd_table_t *vpdtab)
{
    uint8_t *vpddata;
    size_t n;

    vpddata = pciehw_vpd_getdata(phwdev);
    memset(vpddata, 0, PCIEHW_VPDSZ);

    /*
     * Determine size to be sure the data fits in vpddata.
     * If not enough room, we skip vpd data,
     * leaving it filled with zeros.
     */
    n = vpdtab_to_vpddata(vpdtab, NULL);
    if (n > PCIEHW_VPDSZ) {
        pciesys_logerror("%s: vpd_finalize overflow (want %lu)\n",
                         pciehwdev_get_name(phwdev), n);
        return;
    }

    vpdtab_to_vpddata(vpdtab, vpddata);
}

uint32_t
pciehw_vpd_read(pciehwdev_t *phwdev, const uint16_t addr)
{
    if (addr < PCIEHW_VPDSZ) {
        const uint16_t aligned_addr = addr & ~0x3;
        const uint8_t *vpddata = pciehw_vpd_getdata(phwdev);
        const uint32_t data = (((uint32_t)vpddata[aligned_addr + 3] << 24) |
                               ((uint32_t)vpddata[aligned_addr + 2] << 16) |
                               ((uint32_t)vpddata[aligned_addr + 1] <<  8) |
                               ((uint32_t)vpddata[aligned_addr + 0] <<  0));
#ifdef PCIEMGR_DEBUG
        pciesys_logdebug("%s: vpd_read offset 0x%x data 0x%08x\n",
                         pciehwdev_get_name(phwdev), addr, data);
#endif
        return data;
    }
    return 0;
}

void
pciehw_vpd_write(pciehwdev_t *phwdev,
                 const uint16_t addr, const uint32_t data)
{
    /* No writeable vpd data (yet). a*/
}

/******************************************************************
 * debug
 */

/*
 * Dump the vpd info of this device.
 */
static void
vpd_show_dev(pciehwdev_t *phwdev)
{
    const char *name = pciehwdev_get_name(phwdev);
    uint8_t *vpddata = pciehw_vpd_getdata(phwdev);
    int offset;

    pciesys_loginfo("%s vpddata:\n", name);
    for (offset = 0; offset < PCIEHW_VPDSZ; offset += 16) {
        const int n = MIN(16, PCIEHW_VPDSZ - offset);
        char buf[80], *s;

        s = hex_format(buf, sizeof(buf), &vpddata[offset], n);
        pciesys_loginfo("%04x: %s\n", offset, s);
    }
}

void
pciehw_vpd_show(int argc, char *argv[])
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    int opt;
    char *name;
    pciehwdev_t *phwdev;

    name = NULL;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        default:
            return;
        }
    }

    /*
     * No device name specified, show all devs
     */
    if (name == NULL ) {
        int i;

        phwdev = &pshmem->dev[1];
        for (i = 1; i <= pshmem->allocdev; i++, phwdev++) {
            vpd_show_dev(phwdev);
        }
    } else {
        phwdev = pciehwdev_find_by_name(name);
        if (phwdev == NULL) {
            pciesys_logerror("device %s not found\n", name);
            return;
        }
        vpd_show_dev(phwdev);
    }
}
