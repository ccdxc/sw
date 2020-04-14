
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <ctype.h>
#include <dirent.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#include "logger.hpp"

#define CHUNK_SIZE (64 * 1204)
// MIN_SPACE needed is 512MB
uint64_t MIN_SPACE = 512 * 1024 * 1024L;
// MAX_SPACE occupied by core dumps
uint64_t MAX_SPACE = 2 * 1024 * 1024 * 1024L;

struct direntry_t {
    std::string name;
    uint64_t size;
    time_t timestamp;
};

// Give a path make all the directories necessary to get to that path
void mkdirs(const char *dir) {
        char tmp[PATH_MAX];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp), "%s", dir);
        len = strlen(tmp);

        if(tmp[len - 1] == '/')
        {
            tmp[len - 1] = 0;
        }
                
        for(p = tmp + 1; *p; p++)
        {
            if(*p == '/')
            {
                *p = 0;
                mkdir(tmp, S_IRWXU);
                INFO("Creating directory {}", tmp);
                *p = '/';
            }
        }
        INFO("Creating directory {}", tmp);
        mkdir(tmp, S_IRWXU);
}

// Order by timestamp descending
bool direntry_compare_timestamp_desc(direntry_t a, direntry_t b)
{
    return a.timestamp > b.timestamp;
}

// Get all the files in a directory
std::vector<direntry_t> list_dir(char *path)
{
    DIR *path_dir;
    struct dirent *entry;
    std::vector<direntry_t> entries;

    path_dir = opendir(path);
    if (!path_dir) {
        return entries;
    }

    while ((entry = readdir(path_dir)) != NULL) {
        struct stat sb;
        char fullpath[PATH_MAX];
        direntry_t direntry;

        snprintf(fullpath, PATH_MAX, "%s/%s", path, entry->d_name);
        if (stat(fullpath, &sb) != 0 || S_ISDIR(sb.st_mode))
        {
            continue;
        }
        entries.push_back(direntry_t{std::string(fullpath), (uint64_t)sb.st_size, sb.st_ctime});
    }
    closedir(path_dir);

    return entries;
}

// Make sure we are not using more than MAX_SPACE, but we also have at leaste MIN_SPACE
void manage_space(char *path)
{
    struct statvfs sb;
    int rc = statvfs(path, &sb);
    if (rc == -1)
    {
        return;
    }

    auto entries = list_dir(path);
    std::sort(entries.begin(), entries.end(), direntry_compare_timestamp_desc);

    uint64_t free_space = sb.f_bavail * sb.f_frsize;
    uint64_t used_space = 0;
    for (auto e: entries) {
        used_space += e.size;
    }
    INFO("Max: {}, Min: {}", MAX_SPACE, MIN_SPACE);
    INFO("Free: {}, Used: {}", free_space, used_space);
    
    while (((used_space > MAX_SPACE) || (free_space < MIN_SPACE)) && (entries.size() > 0))
    {
        auto e = entries.back();
        entries.pop_back();
        unlink(e.name.c_str());
        free_space += e.size;
        used_space -= e.size;
        INFO("Unlink: {} ({} bytes)", e.name, e.size);
        INFO("Free: {}, Used: {}", free_space, used_space);
    }
}

// Make sure the directory is there and there is enough space
void ensure_path(char *path)
{
    struct stat sb;
    
    if (!(stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)))
    {
        mkdirs(path);
    }

    manage_space(path);
}

// Core dump naming
std::string get_filename(char *path, char *name, pid_t pid)
{
    char timestring[PATH_MAX];
    char filename[PATH_MAX];
    time_t current_time = time(NULL);

    strftime(timestring, PATH_MAX, "%Y%m%d%H%M%S", gmtime(&current_time));
    
    snprintf(filename, PATH_MAX, "%s/core_%s_%i_%s.gz", path, name, pid, timestring);

    return std::string(filename);
}

// Returns root name of the core file, to help coin bundle name.
std::string get_core_file_rootname(std::string filename)
{
    std::string rootname;

    rootname = filename;
    
    rootname.erase(0, rootname.rfind("core_"));
    rootname.erase(rootname.rfind(".gz"), std::string::npos);

    return (rootname);
}

int main(int argc, char *argv[])
{
    int c;
    char *path = NULL;
    char *exec_name = NULL;
    std::string dest_filename;
    std::string core_file_rootname;
    char cmd[PATH_MAX];
    pid_t pid = 0;
    bool bundle_fail;
    int rc;
    
    static struct option long_options[] =
    {         
        {"path",           required_argument, 0, 'P'}, // path to save the core files
        {"pid",            required_argument, 0, 'p'}, // the pid of the cored process
        {"executable",     required_argument, 0, 'e'}, // the executable name of the cored process
        {"max-total-size", required_argument, 0, 'M'}, // the max size(MB) of all files in the core directory
        {"min-free-size",  required_argument, 0, 'm'}, // the min size(MB) that should be availabe in the fs before we start deleting old files
        {0, 0, 0, 0}
    };
    int option_index;

    while ((c = getopt_long(argc, argv, "P:p:e:t:M:m:", long_options, &option_index)) != -1)
    {
        int minmb = 0;
        int maxmb = 0;
        switch (c)
        {
            case 'P':
                path = optarg;
                INFO("Option 'P': {}", path);
                break;
            case 'p':
                pid = atoi(optarg);
                INFO("Option 'p': {}", pid);
                break;
            case 'e':
                exec_name = optarg;
                INFO("Option 'e': {}", exec_name);
                break;
            case 'M':
                maxmb = atoi(optarg);
                INFO("Option 'M': {}", maxmb);
                if (maxmb > 0)
                {
                    MAX_SPACE = maxmb * 1024 * 1024L;
                }
                break;
            case 'm':
                minmb = atoi(optarg);
                INFO("Option 'm': {}", minmb);
                if (minmb > 0)
                {
                    MIN_SPACE = minmb * 1024 * 1024L;
                }
                break;
            default:
                ERR("Unknown argument %c\n", c);
                return 1;
        }
    }

    if (path == NULL) {
        ERR("--path option is required");
        return 1;
    }
    if (pid == 0) {
        ERR("--pid option is required");
        return 1;
    }
    if (exec_name == NULL) {
        ERR("--executable is required");
        return 1;
    }

    ensure_path(path);
    
    dest_filename = get_filename(path, exec_name, pid);
    INFO("Dumping core to {}", dest_filename);
    gzFile out = gzopen(dest_filename.c_str(), "w");
    while (true) {
        char buffer[CHUNK_SIZE];
        ssize_t n = read(STDIN_FILENO, buffer, CHUNK_SIZE);
        if (n <= 0) 
        {
            gzclose(out);
            break;
        }
        gzwrite(out, buffer, n);
    }

    //Create bundle file with core and meta data files.
    bundle_fail = false;
    core_file_rootname = get_core_file_rootname(dest_filename);
    INFO("Creating bundle file {}.tar", core_file_rootname);

    snprintf(cmd, PATH_MAX, "tar -C %s -cf %s/%s.tar %s.gz /nic/etc/VERSION.json > /dev/null 2>&1",
             path, path, core_file_rootname.c_str(), core_file_rootname.c_str());
    INFO("Core tar bundling cmd:{}", cmd);
    rc = system(cmd);
    if ((rc == -1) || !(WIFEXITED(rc))) {
        INFO("Core tar bundle creation failed. rc: {}", rc);
        bundle_fail = true;
    } else {
        if (WEXITSTATUS(rc)) {
            INFO("Core tar bundle creation failed. rc:{}. Error: {}",
                 rc, strerror(WEXITSTATUS(rc)));
            bundle_fail = true;
        }
    }

    if (!bundle_fail) {
        //Remove the base core file.
        snprintf(cmd, PATH_MAX, "rm -f %s > /dev/null 2>&1", dest_filename.c_str());
    } else {
        //Bundle may have gotten created with partial member list. Remove incomplete one.
        INFO(" Keeping core file, removing partial bundle.");
        snprintf(cmd, PATH_MAX, "rm -f %s/%s.tar > /dev/null 2>&1",
                 path, core_file_rootname.c_str());
    }
    rc = system(cmd);
    return 0;
}
