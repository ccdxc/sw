#include <stddef.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cgroups.hpp"
#include "log.hpp"
#include "utils.hpp"

#define CGROUP_ROOT "/sys/fs/cgroup"

#define MAX_FILE_NAME 256

static void
write_to_file (const char *file_name, int number)
{
    FILE *f;

    f = fopen(file_name, "w");
    if (f == NULL) {
        g_log->err("fopen error: %s %d", file_name, errno);
        return;
    }

    fprintf(f, "%i", number);

    fclose(f);
}

void
cg_init (void)
{
    int rc;
    // Mount the memory subsytem
    rc = mount(NULL, CGROUP_ROOT, "cgroup", 0, "memory");
    if (rc == -1) {
        g_log->err("cgroup mount failed: %d", errno);
    }
}

void
cg_create (const char *group_name, size_t mem_size)
{
    char file_name[MAX_FILE_NAME];

    snprintf(file_name, MAX_FILE_NAME, CGROUP_ROOT"/%s", group_name);

    mkdirs(file_name);

    snprintf(file_name, MAX_FILE_NAME,
            CGROUP_ROOT"/%s/memory.limit_in_bytes", group_name);

    write_to_file(file_name, mem_size);
}


void
cg_add (const char *group_name, pid_t pid)
{
    char file_name[MAX_FILE_NAME];

    snprintf(file_name, MAX_FILE_NAME, CGROUP_ROOT"/%s/tasks", group_name);

    write_to_file(file_name, pid);
}

void
cg_reset (pid_t pid)
{
    write_to_file(CGROUP_ROOT"/tasks", pid);
}
