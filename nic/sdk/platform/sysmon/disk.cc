/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"
#include "string.h"
#include <map>

#define DF_COMMAND "df -h | cut -c -80"

#define MB_TO_KB 1024
#define GB_TO_KB (1024 * 1024)

typedef struct mondisk_s {
    string filesystem;
    int64_t size;
    int64_t used;
    int64_t available;
    string mounted;
} mondisk_t;

static std::map<string, mondisk_t> mondisk_map;

/*
 * Convert memory string to integer
*/
static int64_t
convertmemory(string memory) {
    size_t position;
    int64_t number;
    int multiplier = 1;
    if ((position = memory.find("M")) != string::npos) {
        number = stoi(memory.substr(0, position));
        multiplier = MB_TO_KB;
    } else if ((position = memory.find("G")) != string::npos) {
        number = stoi(memory.substr(0, position));
        multiplier = GB_TO_KB;
    } else if ((position = memory.find("K")) != string::npos) {
        number = stoi(memory.substr(0, position));
        multiplier = 1;
    } else {
        number = stoi(memory);
        multiplier = 1;
    }
    return number * multiplier;
}

/*
 * This function checks if the disk info is
 * has changed/new and needs to be logged.
 * The mounted on parameter is used as the key
 * to the map.
*/
static void
checkdiskinfo(mondisk_t *disk) {
    map<string, mondisk_t>::iterator it;

    it = mondisk_map.find(disk->mounted);
    if (it == mondisk_map.end()) {
        SDK_OBFL_TRACE_INFO("%s %f MB, %f MB, %f MB %s",
                disk->filesystem.c_str(), (double)disk->size / 1024.0,
                (double)disk->used / 1024.0,
                (double)disk->available / 1024.0,
                disk->mounted.c_str());
        mondisk_map[disk->mounted] = *disk;
    } else {
        const mondisk_t& old_disk = it->second;
        // Check if any of the sizes have changed.
        if (old_disk.size != disk->size ||
            old_disk.used != disk->used ||
            old_disk.available != disk->available) {
            SDK_OBFL_TRACE_INFO("%s %f MB, %f MB, %f MB %s",
                disk->filesystem.c_str(), (double)disk->size / 1024.0,
                (double)disk->used / 1024.0,
                (double)disk->available / 1024.0,
                disk->mounted.c_str());
            mondisk_map[disk->mounted] = *disk;
        }
    }
}

/**
 * This function takes the output df -h
 * per line and files out the mondisk struct
 */
static int
getdiskinfo(char *psline, mondisk_t *disk) {
    char *token;
    char *newlineremoved;
    vector <string> tokens;

    // Remove the new line character from the line.
    newlineremoved = strtok(psline, "\n");

    token = strtok(newlineremoved, " ");
    while (token != NULL) {
        tokens.push_back(token);
        token = strtok(NULL, " ");
    }

    if (tokens.size() < 6) {
        return -1;
    }
    //do not process the first line
    //skip the filesystem line
    if (tokens[0] == "Filesystem") {
        return -1;
    }
    disk->filesystem = tokens[0];
    disk->size = convertmemory(tokens[1]);
    disk->used = convertmemory(tokens[2]);
    disk->available = convertmemory(tokens[3]);
    disk->mounted = tokens[5];

    return 0;
}

/*
 * Monitor disk space on the system
*/
static void
monitordisk(void) {
    FILE *fptr = NULL;
    char psline[100];

    fptr = popen(DF_COMMAND, "r");
    if (fptr != NULL) {
        while (fgets(psline, sizeof(psline), fptr) != NULL) {
            mondisk_t disk;
            if (getdiskinfo(psline, &disk) >= 0) {
                checkdiskinfo(&disk);
            }
        }
        pclose(fptr);
    }
}

void
checkdisk(void)
{
    static int runtimecounter;

    if (++runtimecounter % 6 == 0) {
        monitordisk();
    }

}

// MONFUNC(checkdisk);
