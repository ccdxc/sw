/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "platform/src/app/pciemgrd/src/pciemgrd_impl.hpp"

/*****************************************************************
 * file ops
 */

static int
writefile(const char *file, const void *buf, const size_t bufsz)
{
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        goto error_out;
    }
    if (fwrite(buf, bufsz, 1, fp) != 1) {
        goto error_out;
    }
    if (fclose(fp) == EOF) {
        goto error_out;
    }
    return bufsz;

 error_out:
    if (fp) (void)fclose(fp);
    (void)unlink(file);
    return -1;
}

static int
copyfile(const char *srcfile, const char *dstfile)
{
    FILE *srcfp, *dstfp;
    size_t n, nbytes;
    char buf[BUFSIZ];

    srcfp = NULL;
    dstfp = NULL;

    srcfp = fopen(srcfile, "r");
    if (srcfp == NULL) {
        goto error_out;
    }
    dstfp = fopen(dstfile, "w");
    if (dstfp == NULL) {
        goto error_out;
    }

    nbytes = 0;
    while ((n = fread(buf, 1, sizeof(buf), srcfp)) > 0) {
        if (fwrite(buf, 1, n, dstfp) != n) {
            goto error_out;
        }
        nbytes += n;
    }
    if (ferror(srcfp)) {
        goto error_out;
    }
    if (fclose(dstfp) == EOF) {
        goto error_out;
    }
    (void)fclose(srcfp);

    return nbytes;

 error_out:
    if (srcfp) fclose(srcfp);
    if (dstfp) fclose(dstfp);
    return -1;
}

/*****************************************************************
 * state save
 */

static int
pcieport_state_save(void)
{
#ifdef __aarch64__
    const char *path = "/data/pcieport_data";
#else
    char path[PATH_MAX];
    const char *hm = getenv("HOME");
    snprintf(path, sizeof(path), "%s/.pcieport_data.sav", hm ? hm : "");
#endif

    // roundup to pagesize because we are going to mmap this file
    const int infosz = roundup(sizeof(pcieport_info_t), getpagesize());
    pcieport_info_t *pi = pcieport_info_get();
    if (pi == NULL) {
        pciesys_logerror("pcieport_state_save: pcieport_info_get failed\n");
        goto error_out;
    }
    if (writefile(path, pi, infosz) != infosz) {
        pciesys_logerror("pcieport_state_save: writefile %s sz %d: %s\n",
                         path, infosz, strerror(errno));
        goto error_out;
    }
    pciesys_loginfo("pcieport_state_save: %s complete sz %d\n", path, infosz);
    return 0;

 error_out:
    return -1;
}

static int
pciemgr_state_save(void)
{
#ifdef __aarch64__
    const char *path = "/data/pciemgr_data";
#else
    char path[PATH_MAX];
    const char *hm = getenv("HOME");
    snprintf(path, sizeof(path), "%s/.pciemgr_data.sav", hm ? hm : "");
#endif

    // roundup to pagesize because we are going to mmap this file
    const int shmemsz = roundup(sizeof(pciehw_shmem_t), getpagesize());
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    if (pshmem == NULL) {
        pciesys_logerror("pciemgr_state_save: pciehw_shmem_get failed\n");
        goto error_out;
    }
    if (writefile(path, pshmem, shmemsz) != shmemsz) {
        pciesys_logerror("pciemgr_state_save: writefile %s sz %d: %s\n",
                         path, shmemsz, strerror(errno));
        goto error_out;
    }
    pciesys_loginfo("pciemgr_state_save: %s complete sz %d\n", path, shmemsz);
    return 0;

 error_out:
    return -1;
}

/*
 * Save the state of the pcie world to prepare for shutdown.
 */
int
upgrade_state_save(void)
{
    pciesys_loginfo("upgrade_state_save: started\n");
    if (pcieport_state_save() < 0) {
        return -1;
    }
    if (pciemgr_state_save() < 0) {
        return -1;
    }
    pciesys_loginfo("upgrade_state_save: completed successfully\n");
    return 0;
}

/*****************************************************************
 * state restore
 */

static int
file_state_restore(const char *srcfile, const char *dstfile, const int filesz)
{
    if (copyfile(srcfile, dstfile) != filesz) {
        /*
         * If the destination file already exists and is the correct size
         * we assume we have restarted again on a system that did not
         * go through an upgrade/restart.  This helps make it easier to
         * test restart without having first to generate a state save event.
         */
        struct stat sb;
        if (stat(dstfile, &sb) >= 0 && sb.st_size == filesz) {
            pciesys_loginfo("file_state_restore: found %s\n", dstfile);
            return 0;
        }
        pciesys_logerror("file_state_restore: "
                         "copyfile %s -> %s failed\n", srcfile, dstfile);
        return -1;
    }
    pciesys_loginfo("file_state_restore: "
                    "copied %s -> %s sz %d\n", srcfile, dstfile, filesz);
    return 0;
}

static int
pcieport_state_restore(void)
{
#ifdef __aarch64__
    const char *srcfile = "/data/pcieport_data";
    const char *dstfile = "/var/run/pcieport_data";
#else
    char srcfile[PATH_MAX];
    char dstfile[PATH_MAX];
    static char *hm = getenv("HOME");
    snprintf(srcfile, sizeof(srcfile), "%s/.pcieport_data.sav", hm ? hm : "");
    snprintf(dstfile, sizeof(dstfile), "%s/.pcieport_data", hm ? hm : "");
#endif
    // roundup to pagesize because we are going to mmap this file
    const int infosz = roundup(sizeof(pcieport_info_t), getpagesize());

    return file_state_restore(srcfile, dstfile, infosz);
}

static int
pciemgr_state_restore(void)
{
#ifdef __aarch64__
    const char *srcfile = "/data/pciemgr_data";
    const char *dstfile = "/var/run/pciemgr_data";
#else
    char srcfile[PATH_MAX];
    char dstfile[PATH_MAX];
    const char *hm = getenv("HOME");
    snprintf(srcfile, sizeof(srcfile), "%s/.pciemgr_data.sav", hm ? hm : "");
    snprintf(dstfile, sizeof(dstfile), "%s/.pciemgr_data", hm ? hm : "");
#endif
    // roundup to pagesize because we are going to mmap this file
    const int shmemsz = roundup(sizeof(pciehw_shmem_t), getpagesize());

    return file_state_restore(srcfile, dstfile, shmemsz);
}

/*
 * We've restarted after a firmware upgrade.  We might be
 * a newer version of the pciemgr but we need to come up and
 * inherit an active server with pcie bus already configured.
 */
int
upgrade_state_restore(void)
{
    pciesys_loginfo("upgrade_state_restore: started\n");
    if (pcieport_state_restore() < 0) {
        return -1;
    }
    if (pciemgr_state_restore() < 0) {
        return -1;
    }
    pciesys_loginfo("upgrade_state_restore: completed successfully\n");
    return 0;
}
