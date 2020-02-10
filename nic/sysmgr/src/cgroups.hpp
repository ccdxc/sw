#ifndef __SYSMGR_CGROUPS_H__
#define __SYSMGR_CGROUPS_H__

#include <stddef.h>
#include <sys/types.h>

#define MAX_CPUSHARES 1024  // Max value per cgroup definitions.
// Different cgroup types that we support
typedef enum cg_type_ {
    CG_MEMORY = 0,      //Maps "memory-limit" from .json config.
    CG_CPU    = 1,      //Maps "cpu-max-percentage" from .json confing.
    CG_CPUSET = 2,      //Maps "cpuset" from .json config.
    CG_MAX_VALUE  = CG_CPUSET
} cg_type_t;
#define MAX_CG_COUNT (CG_MAX_VALUE + 1)

typedef union cg_limit_ {
    size_t      memory;
    int         cpu_shares;
    const char  *cpuset;
} cg_limit_t;

void cg_init(void);
void cg_create (cg_type_t type, const char *group_name, cg_limit_t limit);
void cg_add (cg_type_t type, const char *group_name, pid_t pid);
void cg_reset(cg_type_t type, pid_t pid);

#endif //__SYSMGR_CGROUPS_H__
