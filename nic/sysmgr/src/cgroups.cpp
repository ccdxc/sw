#include <stddef.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cgroups.hpp"
#include "log.hpp"
#include "utils.hpp"

//Top level mount dir for differrent types of cgroup controllers.
#define CGROUP_ROOT "/sys/fs/cgroup"
#define MAX_FILE_NAME 256

// Bit mask definitions to track initialisation state of different
// types of cgroups.
#define CG_STATE_MEMORY       0x01
#define CG_STATE_CPU          0x02
#define CG_STATE_CPUSET       0x04
#define CG_STATE_ALL          (CG_STATE_MEMORY | CG_STATE_CPU | \
                               CG_STATE_CPUSET)
static uint cg_state  = 0;  //Bit map to track initialisation state of
                            //different cgroup types (controllers).

// Typedef for describing a cgroup of interest.
typedef struct cgroup_ {
    cg_type_t cg_type;
    uint cg_state_mask;       //CG_STATE_XX bit representation, to track
                              //group's init. state into 'cg_state'.
    char *target_dir;         //Target dir to mount this cgroup type into.
    char *cg_type_name;       //cgroup type (ex. memory/cpu/cpuset/...).
} cgroup_t;

static char cg_memory_target[] = "/sys/fs/cgroup/memory";
static char cg_cpu_target[] = "/sys/fs/cgroup/cpu";
static char cg_cpuset_target[] = "/sys/fs/cgroup/cpuset";
static char cg_mem_name[] = "memory";
static char cg_cpu_name[] = "cpu";
static char cg_cpuset_name[] = "cpuset";

// List of all cgroups types that we support.
static cgroup_t cg_list[MAX_CG_COUNT] {
    {CG_MEMORY, CG_STATE_MEMORY, cg_memory_target, cg_mem_name},
    {CG_CPU,    CG_STATE_CPU,    cg_cpu_target,    cg_cpu_name},
    {CG_CPUSET, CG_STATE_CPUSET, cg_cpuset_target, cg_cpuset_name},
};

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

//Writes limiter value to a specific group identified by the 'file_name'
//parameter
static void
write_cg_limit (cg_type_t type, const char *file_name, cg_limit_t limit)
{
    FILE *f;

    f = fopen(file_name, "w");
    if (f == NULL) {
        g_log->err("fopen error: %s %d", file_name, errno);
        return;
    }

    switch (type) {
    case CG_MEMORY:
        fprintf(f, "%lu", limit.memory);
        break;
    case CG_CPU:
        fprintf(f, "%i", limit.cpu_shares);
        break;
    case CG_CPUSET:
        fprintf(f, "%s", limit.cpuset);
        break;
    default:
        break;
    }

    fclose(f);
}

//Mounts a cgroup of type specified into respective target directory
//and sets it's initialisation state.
void cg_mount (cg_type_t cg_type)
{
    int rc;

    mkdirs(cg_list[cg_type].target_dir);
    rc = mount(NULL, cg_list[cg_type].target_dir, "cgroup", 0,
               cg_list[cg_type].cg_type_name);
    if (rc == -1) {
        g_log->err("cgroup mount (type: %s) failed: %d",
                   cg_list[cg_type].cg_type_name, errno);
        cg_state &= !(cg_list[cg_type].cg_state_mask);
    } else {
        cg_state |= cg_list[cg_type].cg_state_mask;
    }
}

//Mounts each type of cgroup (memory, cpu etc) into respective, dedicated
//directories under the toplevel root cgroup directory CGROUP_ROOT. 
void
cg_init (void)
{
    int rc;
    int idx;
    char file_name[MAX_FILE_NAME];

    cg_state = 0;
    // Mount the filesystem for cgroup management.
    rc = mount(NULL, CGROUP_ROOT, "tmpfs", 0, "");
    if (rc == -1) {
        g_log->err("cgroup mount failed: %d", errno);
        return;
    }

    // Mount each type of cgroup (controller type) into respective
    // top level directories.
    for (idx = 0; idx < MAX_CG_COUNT; idx++) {
        cg_mount((cg_type_t)idx);
    }

    if ((cg_state & CG_STATE_ALL) != CG_STATE_ALL) {
        g_log->info("One or more cgroup controller inits failed."); 
    } else {
        g_log->info("All cgroup controllers mounted successfully.");
    }

    // Workaround for the apparent known issue with 'cpuset'
    // cgroup type which results in write error (throwing insufficient
    // space error) for any sub-group that gets created under the
    // parent). Workaound is to write a value of 1 to the file
    // 'cgroup.clone_children' under top level cgroup directory.
    snprintf(file_name, MAX_FILE_NAME, "%s/%s",
             cg_list[CG_CPUSET].target_dir, "cgroup.clone_children");
    write_to_file(file_name, 1);
}


// 'group_name' is the process name. We create a specific sub-group 
// per process name, per type of cgroup controller (memory,cpu, cpuset) for
// each of the process that has turned on control.
// For example:
// if HAL has all three of memory, cpu-max-percentage, cpuset turned on,
// there would be three cgroups created (as shown below) for HAL process
// alone - one for each cgroup type:
//      /sys/fs/cgroup/memory/hal
//      /sys/fs/cgroup/cpu/hal
//      /sys/fs/cgroup/cpuset/hal
//
void
cg_create (cg_type_t type, const char *group_name, cg_limit_t limit)
{
    char file_name[MAX_FILE_NAME];

    snprintf(file_name, MAX_FILE_NAME, "%s/%s",
             cg_list[type].target_dir, group_name);

    //Create the cgroup i.e. just create the dir for the group name in 
    //question.
    mkdirs(file_name);

    switch (type) {
    case CG_MEMORY:
        snprintf(file_name, MAX_FILE_NAME,
                 "%s/%s/memory.limit_in_bytes",
                 cg_list[type].target_dir, group_name);
        break;
    case CG_CPU:
        snprintf(file_name, MAX_FILE_NAME,
                 "%s/%s/cpu.shares",
                 cg_list[type].target_dir, group_name);
        break;
    case CG_CPUSET:
        snprintf(file_name, MAX_FILE_NAME,
                 "%s/%s/cpuset.cpus",
                 cg_list[type].target_dir, group_name);
        break;
    default:
        break;
    }

    write_cg_limit(type, file_name, limit);
}

//Note: 'group_name' is the process name. 
//Puts pid into the cgroup that has gotten created for that specific
//process name.
void
cg_add (cg_type_t type, const char *group_name, pid_t pid)
{
    char file_name[MAX_FILE_NAME];

    snprintf(file_name, MAX_FILE_NAME, "%s/%s/cgroup.procs",
             cg_list[type].target_dir, group_name);

    write_to_file(file_name, pid);
}

// Remove the pid from the cgroup of the type speicified i.e. write the
// pid to the top level directory of the concerned cgroup type.
void
cg_reset (cg_type_t type, pid_t pid)
{
    char file_name[MAX_FILE_NAME];

    snprintf(file_name, MAX_FILE_NAME, "%s/cgroup.procs",
             cg_list[type].target_dir);
    write_to_file(file_name, pid);
}
