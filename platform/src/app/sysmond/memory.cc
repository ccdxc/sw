/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "string.h"
#include <map>

#define PS_COMMAND "ps -o pid,rss,vsz,comm,args | grep -v \"\\[\""
#define MEMINFO "/proc/meminfo"
#define TOTAL_MEMORY "MemTotal"
#define FREE_MEMORY "MemFree"
#define AVAILABLE_MEMORY "MemAvailable"

#define AVAILABLE_MEMORY_THRESHOLD (16 * 1024)
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

static delphi::objects::asicmemorymetrics_t asicmemory;
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
        TRACE_INFO(GetObflLogger(), "{}({}): RSS: {} KB, VSZ: {} KB",
                process->command, process->pid, process->rss, process->vsz);
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
                TRACE_INFO(GetObflLogger(), "{}({}):RSS: {} KB (+{} KB), " \
                           "VSZ: {} KB (+{} KB)",
                            process->command, process->pid,
                            process->rss, process->rss_change,
                            process->vsz, process->vsz_change);
                //Reinitialize rss/vsz change to 0.
                process->rss_change = 0;
                process->vsz_change = 0;
            } else if (process->rss_change >= PROCESS_CHANGE_THRESHOLD) {
                //log the change in RSS
                TRACE_INFO(GetObflLogger(), "{}({}):RSS: {}KB (+{}KB)",
                            process->command, process->pid,
                            process->rss, process->rss_change);
                //Reinitialize rss_change  to 0
                process->rss_change = 0;
            } else if (process->vsz_change >= PROCESS_CHANGE_THRESHOLD) {
                //log the change in vsz
                TRACE_INFO(GetObflLogger(), "{}({}):VSZ: {}KB (+{}KB)",
                            process->command, process->pid,
                            process->vsz, process->vsz_change);
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
    //skip the ps process
    if (tokens[0] == "PID" ||
        tokens[3] == "ps") {
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
            TRACE_INFO(GetObflLogger(), "{}({}) - exited",
                       cur_process.command, cur_process.pid);
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
monitorfreememory(void) {
    FILE *fptr = NULL;
    char line[100];
    string meminfoline;
    size_t found;
    int64_t curr_memory;
    static int64_t mem_diff;
    static int64_t avail_memory_lowest = INT64_MAX;

    fptr = fopen(MEMINFO, "r");
    if (fptr != NULL) {
        while (fgets(line, sizeof(line), fptr) != NULL) {
            meminfoline = line;
            if ((found = meminfoline.find(TOTAL_MEMORY)) != string::npos) {
                asicmemory.totalmemory = getmeminfo(line);
            }
            if ((found = meminfoline.find(AVAILABLE_MEMORY)) != string::npos) {
                curr_memory = getmeminfo(line);
                if (asicmemory.availablememory == 0) {
                    avail_memory_lowest = curr_memory;
                    TRACE_INFO(GetObflLogger(), "Available memory {} KB", avail_memory_lowest);
                } else if (curr_memory < avail_memory_lowest) {
                    mem_diff = mem_diff + avail_memory_lowest - curr_memory;
                    avail_memory_lowest = curr_memory;
                    if (mem_diff >= AVAILABLE_MEMORY_THRESHOLD) {
                        TRACE_INFO(GetObflLogger(), "Available memory lowerwatermark {} KB", avail_memory_lowest);
                        mem_diff = 0;
                    }
                }
                asicmemory.availablememory = curr_memory;
            }
            if ((found = meminfoline.find(FREE_MEMORY)) != string::npos) {
                asicmemory.freememory = getmeminfo(line);
            }
        }
        pclose(fptr);
    }
}

/*
 * Monitor process on the system
*/
static void
monitorprocess(void) {
    FILE *fptr = NULL;
    char cmd[80];
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
    }
}

static void
checkmemory(void)
{
    uint64_t key = 0;

    //monitor the processes in system
    monitorprocess();
    //remove the pids which are no longer used.
    removeprocess();
    //monitor the available curr_memory
    monitorfreememory();
    //Publish Delphi object
    delphi::objects::AsicMemoryMetrics::Publish(key, &asicmemory);
    color = !color;
    return;
}

MONFUNC(checkmemory);
