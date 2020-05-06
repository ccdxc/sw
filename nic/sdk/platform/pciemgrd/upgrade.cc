/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <cinttypes>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pciemgrd/pciemgrd_impl.hpp"

/*****************************************************************
 * upgrade design notes:
 *
 * pciemgr has runtime state in
 *
 *     /var/run/pcieport_data
 *     /var/run/pciemgr_data
 *
 * For an upgrade, we will save this state in persistent storage
 * by copying to
 *
 *     /update/pcieport_upgdata
 *     /update/pciemgr_upgdata
 *
 * The normal sequence of events for upgrade looks like this:
 *
 *     (1) PciemgrSvcHandler::CompatCheckHandler()
 *     (2) PciemgrSvcHandler::SaveStateHandler()
 *     (3) kill all processes including pciemgrd
 *     (4) switchroot to new root fs with upgraded fw image
 *     (5) start up all processes as normal
 *     (6) pciemgrd detects at startup saved state files
 *     (7) pciemgrd restores/inherits saved state files
 *     (8) PciemgrSvcHandler::SuccessHandler()
 *
 * If we find saved files present on startup (6) then we know we are in
 * the process of doing an upgrade and we restore the state from
 * persistent storage to the runtime path and re-load it from there (7).
 *
 * ----------------
 * Failed Upgrades
 * ----------------
 *
 * If there is an error during the upgrade process then things get
 * interesting.  We want to get back to the state where we started
 * and how we get back depends on how far down the upgrade path we got
 * before the failure.
 *
 * If we are in the CompatCheck stage then all processes continue
 * as before--no need to save/restore any state.
 *
 * If we are in the pre-switchroot stage then we are still running
 * the original version of fw.  We begin a rollback and when pciemgrd
 * restarts it restores the saved state.
 *
 * If we are in the post-switchroot stage then we have likely restored
 * the persistent saved to the new running state, possibly transforming
 * the state to the format compatible with the new fw.  For that case,
 * we are careful to leave the persistent saved state in place.  The
 * saved state was created by the original fw version so it will always
 * be compatible with the original fw.  Keeping saved state around in
 * the original format eliminates the need for new fw to recreate
 * state in any (all?) previous format(s).  A post-switchroot rollback
 * is then similar to a pre-switchroot rollback.  pciemgrd restarts
 * after a switchroot back to the original fw, restores the saved state,
 * and continues.
 */

/*****************************************************************
 * manage data file paths
 */

static const char *rundir = "/var/run";
static const char *savedir = "/update";

static const char *pcieport_runfile  = "pcieport_data";
static const char *pciemgr_runfile   = "pciemgr_data";

static const char *pcieport_savefile = "pcieport_upgdata";
static const char *pciemgr_savefile  = "pciemgr_upgdata";
static const char *rollback_savefile = "pciemgr_upgrollback";

static char *
file_path(const char *dir, const char *file, char *path, const size_t pathsz)
{
#ifdef __aarch64__
    snprintf(path, pathsz, "%s/%s", dir, file);
#else
    snprintf(path, pathsz, "%s/.%s", getenv("HOME"), file);
#endif
    return path;
}

static char *
run_path(const char *file)
{
    static char path[PATH_MAX];
    return file_path(rundir, file, path, sizeof(path));
}

static char *
save_path(const char *file)
{
    static char path[PATH_MAX];
    return file_path(savedir, file, path, sizeof(path));
}

static char *
pcieport_runpath(void)
{
    return run_path(pcieport_runfile);
}

static char *
pcieport_savepath(void)
{
    return save_path(pcieport_savefile);
}

static char *
pciemgr_runpath(void)
{
    return run_path(pciemgr_runfile);
}

static char *
pciemgr_savepath(void)
{
    return save_path(pciemgr_savefile);
}

static char *
rollback_savepath(void)
{
    return save_path(rollback_savefile);
}

/*****************************************************************
 * file ops
 */

static int
readfile(const char *file, void *buf, const size_t bufsz)
{
    int n;
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        goto error_out;
    }
    n = fread(buf, 1, bufsz, fp);
    if (ferror(fp)) {
        goto error_out;
    }
    (void)fclose(fp);
    return n;

 error_out:
    if (fp) (void)fclose(fp);
    return -1;
}

static int
writefile(const char *file, const void *buf, const size_t bufsz)
{
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        goto error_out;
    }
    if (bufsz > 0 && fwrite(buf, bufsz, 1, fp) != 1) {
        goto error_out;
    }
    if (fclose(fp) == EOF) {
        fp = NULL;
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
        dstfp = NULL;
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
    const char *path = pcieport_savepath();

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
    const char *path = pciemgr_savepath();

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

/*****************************************************************
 * state restore
 */

static int
file_state_restore(const char *srcfile, const char *dstfile, const int filesz)
{
    int copysz;

    copysz = copyfile(srcfile, dstfile);
    if (copysz < 0) {
        /*
         * Before we fail this operation check if the destination
         * file already exists and is the correct size, if so then
         * we assume we have restarted again on a system that did not
         * go through an upgrade/restart.  This helps make it easier to
         * test restart without having first to generate a state save event.
         */
        struct stat sb;
        if (stat(dstfile, &sb) >= 0 && sb.st_size == filesz) {
            pciesys_loginfo("file_state_restore: found %s\n", dstfile);
            return 0;
        }
        pciesys_logerror("file_state_restore: copyfile %s -> %s %d failed\n",
                         srcfile, dstfile, filesz);
        return -1;
    }
    pciesys_loginfo("file_state_restore: "
                    "copied %s -> %s copysz %d filesz %d\n",
                    srcfile, dstfile, copysz, filesz);
    return 0;
}

static int
pcieport_state_restore(void)
{
    const char *srcfile = pcieport_savepath();
    const char *dstfile = pcieport_runpath();

    // roundup to pagesize because we are going to mmap this file
    const int infosz = roundup(sizeof(pcieport_info_t), getpagesize());

    return file_state_restore(srcfile, dstfile, infosz);
}

static int
pciemgr_state_restore(void)
{
    const char *srcfile = pciemgr_savepath();
    const char *dstfile = pciemgr_runpath();

    // roundup to pagesize because we are going to mmap this file
    const int shmemsz = roundup(sizeof(pciehw_shmem_t), getpagesize());

    return file_state_restore(srcfile, dstfile, shmemsz);
}

/*****************************************************************
 * upgrade apis
 */

/*
 * Save the state of the pcie world to prepare for firmware upgrade.
 */
int
upgrade_state_save(void)
{
    pciesys_loginfo("upgrade_state_save: started\n");
    if (pcieport_state_save() < 0) {
        return -1;
    }
    if (pciemgr_state_save() < 0) {
        (void)unlink(pcieport_savepath());
        return -1;
    }
    pciesys_loginfo("upgrade_state_save: completed successfully\n");
    return 0;
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

static int
upgrade_state_delete(void)
{
    (void)unlink(pcieport_savepath());
    (void)unlink(pciemgr_savepath());
    return 0;
}

//
// Check if an upgrade is currently in progress.
//
int
upgrade_in_progress(void)
{
    // check for existence of the saved files
    // XXX could also check sizes
    return (access(pcieport_savepath(), R_OK) == 0 &&
            access(pciemgr_savepath(), R_OK) == 0);
    if (0) readfile(NULL, NULL, 0);
}

//
// Upgrade complete, delete any saved state since we are done with it.
//
int
upgrade_complete(void)
{
    pciesys_loginfo("upgrade_complete\n");
    return upgrade_state_delete();
}

//
// Upgrade failed, we don't need to keep any save state around.
//
int
upgrade_failed(void)
{
    pciesys_loginfo("upgrade_failed\n");
    return upgrade_state_delete();
}

//
// Upgrade failed, but we need to keep save state and mark
// that rollback is in progress so we can detected this
// condition after process restart (possible after switchroot).
int
upgrade_rollback_begin(void)
{
    pciesys_loginfo("upgrade_rollback_begin:\n");
    return writefile(rollback_savepath(), NULL, 0);
}

//
// Rollback is complete.  After a failed upgrade we initiated
// a rollback to the previous version of fw and restored the
// original state.  Now we are done with the original saved state.
//
int
upgrade_rollback_complete(void)
{
    pciesys_loginfo("upgrade_rollback_complete:\n");
    upgrade_state_delete();
    (void)unlink(rollback_savepath());
    return 0;
}

//
// Check if a rollback is currently in progress.
//
int
upgrade_rollback_in_progress(void)
{
    return access(rollback_savepath(), R_OK) == 0;
}
