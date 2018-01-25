#ifndef _LKL_LINUX_SHM_H_
#define _LKL_LINUX_SHM_H_

#include <lkl/linux/ipc.h>
#include <lkl/linux/errno.h>
#include <unistd.h>

/*
 * LKL_SHMMNI, LKL_SHMMAX and LKL_SHMALL are default upper limits which can be
 * modified by sysctl. The LKL_SHMMAX and LKL_SHMALL values have been chosen to
 * be as large possible without facilitating scenarios where userspace
 * causes overflows when adjusting the limits via operations of the form
 * "retrieve current limit; add X; update limit". It is therefore not
 * advised to make LKL_SHMMAX and LKL_SHMALL any larger. These limits are
 * suitable for both 32 and 64-bit systems.
 */
#define LKL_SHMMIN 1			 /* min shared seg size (bytes) */
#define LKL_SHMMNI 4096			 /* max num of segs system wide */
#define LKL_SHMMAX (ULONG_MAX - (1UL << 24)) /* max shared seg size (bytes) */
#define LKL_SHMALL (ULONG_MAX - (1UL << 24)) /* max shm system wide (pages) */
#define LKL_SHMSEG LKL_SHMMNI			 /* max shared segs per process */

/* Obsolete, used only for backwards compatibility and libc5 compiles */
struct lkl_shmid_ds {
	struct ipc_perm		shm_perm;	/* operation perms */
	int			shm_segsz;	/* size of segment (bytes) */
	__lkl__kernel_time_t		shm_atime;	/* last attach time */
	__lkl__kernel_time_t		shm_dtime;	/* last detach time */
	__lkl__kernel_time_t		shm_ctime;	/* last change time */
	__lkl__kernel_ipc_pid_t	shm_cpid;	/* pid of creator */
	__lkl__kernel_ipc_pid_t	shm_lpid;	/* pid of last operator */
	unsigned short		shm_nattch;	/* no. of current attaches */
	unsigned short 		shm_unused;	/* compatibility */
	void 			*shm_unused2;	/* ditto - used by LKL_DIPC */
	void			*shm_unused3;	/* unused */
};

/* Include the definition of shmid64_ds and shminfo64 */
#include <lkl/asm/shmbuf.h>

/* permission flag for shmget */
#define LKL_SHM_R		0400	/* or S_IRUGO from <linux/stat.h> */
#define LKL_SHM_W		0200	/* or S_IWUGO from <linux/stat.h> */

/* mode for attach */
#define	LKL_SHM_RDONLY	010000	/* read-only access */
#define	LKL_SHM_RND		020000	/* round attach address to SHMLBA boundary */
#define	LKL_SHM_REMAP	040000	/* take-over region on attach */
#define	LKL_SHM_EXEC	0100000	/* execution access */

/* super user shmctl commands */
#define LKL_SHM_LOCK 	11
#define LKL_SHM_UNLOCK 	12

/* ipcs ctl commands */
#define LKL_SHM_STAT 	13
#define LKL_SHM_INFO 	14

/* Obsolete, used only for backwards compatibility */
struct	lkl_shminfo {
	int shmmax;
	int shmmin;
	int shmmni;
	int shmseg;
	int shmall;
};

struct lkl_shm_info {
	int used_ids;
	__lkl__kernel_ulong_t shm_tot;	/* total allocated shm */
	__lkl__kernel_ulong_t shm_rss;	/* total resident shm */
	__lkl__kernel_ulong_t shm_swp;	/* total swapped shm */
	__lkl__kernel_ulong_t swap_attempts;
	__lkl__kernel_ulong_t swap_successes;
};


#endif /* _LKL_LINUX_SHM_H_ */
