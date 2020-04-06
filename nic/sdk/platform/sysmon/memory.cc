/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"
#include "string.h"
#include <map>

#define PS_COMMAND "ps -o pid,rss,vsz,comm,args | grep -v \"\\[\" | cut -c -80"
#define MEMINFO "/proc/meminfo"
#define TOTAL_MEMORY "MemTotal"
#define FREE_MEMORY "MemFree"
#define AVAILABLE_MEMORY "MemAvailable"

#define AVAILABLE_MEMORY_THRESHOLD (16 * 1024)
#define LOW_MEMORY_THRESHOLD (500 * 1024)
#define CRITICAL_MEMORY_THRESHOLD (100 * 1024)
#define PROCESS_CHANGE_THRESHOLD 1024
#define MB_TO_KB 1024
#define GB_TO_KB (1024 * 1024)

typedef struct monprocess_s {
    int pid;
    string command;
    bool visited;
    int64_t rss;
    int64_t vsz;
    int64_t rss_change;
    int64_t vsz_change;
} monprocess_t;

static std::map<int, monprocess_t> monprocess_map;
static bool color = true;

/*
 * This function checks if the process is
 * a new process or an old process. If an
 * old process then it compares the memory
 * (vsz and rss) to see if any change has 
 * happened.
*/
static void
checkprocess(monprocess_t *process) {
    map<int, monprocess_t>::iterator it;

    it = monprocess_map.find(process->pid);
    if (it == monprocess_map.end()) {
        SDK_HMON_TRACE_ERR("%s(%u): RSS: %f MB, VSZ: %f MB",
                process->command.c_str(), process->pid, (double)process->rss / 1024.0,
                (double)process->vsz / 1024.0);
        monprocess_map[process->pid] = *process;
    } else {
        const monprocess_t& old_process = it->second;
        //Check if the PID belongs to the same process
        if (old_process.command == process->command) {
            process->rss_change = old_process.rss_change +
                                (process->rss - old_process.rss);
            process->vsz_change = old_process.vsz_change +
                                (process->vsz - old_process.vsz);
            if (process->rss_change >= PROCESS_CHANGE_THRESHOLD &&
                process->vsz_change >= PROCESS_CHANGE_THRESHOLD) {
                //log the change in RSS
                SDK_HMON_TRACE_ERR("%s(%u):RSS: %f MB (+%f MB), " \
                           "VSZ: %f MB (+%f MB)",
                            process->command.c_str(), process->pid,
                            (double)process->rss / 1024.0,
                            (double)process->rss_change / 1024.0,
                            (double)process->vsz / 1024.0,
                            (double)process->vsz_change / 1024.0);
                //Reinitialize rss/vsz change to 0.
                process->rss_change = 0;
                process->vsz_change = 0;
            } else if (process->rss_change >= PROCESS_CHANGE_THRESHOLD) {
                //log the change in RSS
                SDK_HMON_TRACE_ERR("%s(%u):RSS: %fMB (+%fMB)",
                            process->command.c_str(), process->pid,
                            (double)process->rss / 1024.0,
                            (double)process->rss_change / 1024.0);
                //Reinitialize rss_change  to 0
                process->rss_change = 0;
            } else if (process->vsz_change >= PROCESS_CHANGE_THRESHOLD) {
                //log the change in vsz
                SDK_HMON_TRACE_ERR("%s(%u):VSZ: %fMB (+%fMB)",
                            process->command.c_str(), process->pid,
                            (double)process->vsz / 1024.0,
                            (double)process->vsz_change / 1024.0);
                //Reinitialize vsz_change  to 0
                process->vsz_change = 0;
            }
            monprocess_map[process->pid] = *process;
        } else {
            //Reused PID reassign map entry
            monprocess_map[process->pid] = *process;
        }
    }
}

/*
 * Convert memory string to integer
*/
static int64_t
convertmemory(string memory) {
    size_t position;
    int64_t number;
    int multiplier = 1;
    if ((position = memory.find("m")) != string::npos) {
        number = stoi(memory.substr(0, position));
        multiplier = MB_TO_KB;
    } else if ((position = memory.find("g")) != string::npos) {
        number = stoi(memory.substr(0, position));
        multiplier = GB_TO_KB;
    } else {
        number = stoi(memory);
        multiplier = 1;
    }
    return number * multiplier;
}

/*
 * This function parses the output of
 * ps which is passed to it per line.
 * It ignores the first line and the ps
 * process itself.
*/
static int
getMemory(char *psline, monprocess_t *process) {
    char *token;
    vector <string> tokens;

    token = strtok(psline, " ");
    while (token != NULL) {
        tokens.push_back(token);
        token = strtok(NULL, " ");
    }

    if (tokens.size() < 4) {
        return -1;
    }
    //do not process the first line
    //skip the ps,sleep and cut processes
    if (tokens[0] == "PID" ||
        tokens[3] == "ps" ||
        tokens[3] == "cut" ||
        tokens[3] == "sleep") {
        return -1;
    }
    process->command = tokens[3];
    process->pid = stoi(tokens[0]);
    process->rss = convertmemory(tokens[1]);
    process->vsz = convertmemory(tokens[2]);

    return 0;
}

/*
 * This function removes the processes which
 * have been killed since last time.
*/
static void
removeprocess() {
    map<int, monprocess_t>::iterator it;
    vector <int> removeitem;

    for (it = monprocess_map.begin(); it != monprocess_map.end(); it++) {
        const monprocess_t &cur_process = it->second;
        if (cur_process.visited != color) {
            SDK_HMON_TRACE_ERR("%s(%u) - exited",
                       cur_process.command.c_str(), cur_process.pid);
            removeitem.push_back(it->first);
        }
    }

    for (auto &i : removeitem) {
        monprocess_map.erase(i);
    }
}

/*
 * This function parses the output of /proc/meminfo
 * to get the totalmemory, availablememory and
 * freememory.
*/
static int64_t
getmeminfo(char *psline) {
    char *token;
    vector <string> tokens;

    token = strtok(psline, " ");
    while (token != NULL) {
        tokens.push_back(token);
        token = strtok(NULL, " ");
    }

    return convertmemory(tokens[1]);
}

/*
 * Monitor free curr_memory on the system
*/
static void
monitorfreememory (uint64_t *total_mem, uint64_t *available_mem, 
                   uint64_t *free_mem) {
    FILE *fptr = NULL;
    char line[100];
    string meminfoline;
    size_t found;
    int64_t curr_memory;
    static bool sysmondebug = true;
    static int64_t mem_diff;
    static int64_t avail_memory_lowest = INT64_MAX;

    fptr = fopen(MEMINFO, "r");
    if (fptr != NULL) {
        while (fgets(line, sizeof(line), fptr) != NULL) {
            meminfoline = line;
            if ((found = meminfoline.find(TOTAL_MEMORY)) != string::npos) {
                *total_mem = getmeminfo(line);
            }
            if ((found = meminfoline.find(AVAILABLE_MEMORY)) != string::npos) {
                curr_memory = getmeminfo(line);
                if (*available_mem == 0) {
                    avail_memory_lowest = curr_memory;
                    SDK_HMON_TRACE_ERR("Available memory %f MB",
                    (double)avail_memory_lowest / 1024.0);
                } else if (curr_memory < avail_memory_lowest) {
                    mem_diff = mem_diff + avail_memory_lowest - curr_memory;
                    avail_memory_lowest = curr_memory;
                    if (mem_diff >= AVAILABLE_MEMORY_THRESHOLD) {
                        SDK_HMON_TRACE_ERR("Available memory lowerwatermark %f MB",
                        (double)avail_memory_lowest / 1024.0);
                        mem_diff = 0;
                    }
                }
                if (curr_memory < LOW_MEMORY_THRESHOLD) {
                    SDK_HMON_TRACE_ERR("Available memory is %f MB",
                    (double)curr_memory / 1024.0);
                }

                if (sysmondebug == false &&
                    curr_memory > CRITICAL_MEMORY_THRESHOLD) {
                    // Enable the sysmonddebug script again.
                    SDK_HMON_TRACE_ERR("Enable sysmonddebug script again");
                    sysmondebug = true;
                }

                if (sysmondebug == true &&
                    curr_memory < CRITICAL_MEMORY_THRESHOLD) {
                    int status = system("/nic/tools/sysmondebug.sh");
                    if (status) {
                        SDK_HMON_TRACE_ERR("Unable to run debug script",
                            (double)curr_memory / 1024.0);
                    } else {
                        SDK_HMON_TRACE_ERR("Available memory is %f MB, run debug script",
                            (double)curr_memory / 1024.0);
                        sysmondebug = false;
                    }
                }
                *available_mem = curr_memory;
            }
            if ((found = meminfoline.find(FREE_MEMORY)) != string::npos) {
                *free_mem = getmeminfo(line);
            }
        }
        fclose(fptr);
    }
}

/*
 * Monitor process on the system.
 * Return false when PS command fails
*/
static bool
monitorprocess(void) {
    FILE *fptr = NULL;
    char psline[100];

    fptr = popen(PS_COMMAND, "r");
    if (fptr != NULL) {
        while (fgets(psline, sizeof(psline), fptr) != NULL) {
            monprocess_t process;
            process.visited = color;
            process.rss_change = 0;
            process.vsz_change = 0;
            if (getMemory(psline, &process) >= 0) {
                checkprocess(&process);
            }
        }
        pclose(fptr);
        return true;
    }
    return false;
}

void
checkmemory(void)
{
    static int runtimecounter;
    static uint64_t total_mem = 0;
    static uint64_t available_mem = 0;
    static uint64_t free_mem = 0;

    if (++runtimecounter < 6) {
        return;
    }
    //monitor the processes in system
    if (monitorprocess()) {
        //remove the pids which are no longer used.
        removeprocess();
        color = !color;
    }

    //monitor the available curr_memory
    monitorfreememory(&total_mem, &available_mem, &free_mem);
    if (g_sysmon_cfg.memory_event_cb) {
        g_sysmon_cfg.memory_event_cb(total_mem, available_mem, free_mem);
    }

}

// MONFUNC(checkmemory);
