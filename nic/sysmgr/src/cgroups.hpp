#ifndef __SYSMGR_CGROUPS_H__
#define __SYSMGR_CGROUPS_H__

#include <stddef.h>
#include <sys/types.h>

void cg_init(void);
void cg_create(const char *group_name, size_t mem_size);
void cg_add(const char *group_name, pid_t pid);
void cg_reset(pid_t pid);

#endif //__SYSMGR_CGROUPS_H__
