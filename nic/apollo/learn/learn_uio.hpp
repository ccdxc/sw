//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __LEARN_UIO_HPP__
#define __LEARN_UIO_HPP__

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/learn.hpp"

using namespace std;

static bool
uio_device_ready_hw (void)
{
    struct dirent *de;
    DIR *dr = opendir(UIO_DEV_ROOT);
    std::string uio_dev, file_content, dev_file;
    std::ifstream ifs;
    bool ret = false;

    if (dr == NULL) {
        return false;
    }

    while ((de = readdir(dr)) != NULL) {
        uio_dev = UIO_DEV_ROOT;
        uio_dev += de->d_name;
        uio_dev += "/name";
        ifs.open(uio_dev, std::ifstream::in);
        if (!ifs.is_open()) {
            continue;
        }
        ifs >> file_content;
        ifs.close();
        if (0 == file_content.compare(LEARN_UIO_DEV_NAME)) {
            dev_file = "/dev/";
            dev_file += de->d_name;
            int fd = open(dev_file.c_str(), O_RDWR);
            if (fd < 0) {
                ret = false;
                break;
            }
            // though uio device open succeeds, in rare cases its seen that
            // all files under device aren't populated, so dpdk init fails.
            // there is niether any good way to check if all files are
            // populated nor any notification mechanism in linux to notify
            // that a uio device is created. so we rely on adding a delay here.
            sleep(3);
            ret = true;
            close(fd);
            break;
        }
    }

    closedir(dr); 
    return ret;
}

static bool
uio_device_ready_sim (void)
{
    char *topdir;
    string device_file, line;
    ifstream ifs;
    size_t found_pos;
    bool found = false;

    topdir = std::getenv("PDSPKG_TOPDIR");
    if (!topdir) {
        return false;
    }

    device_file = topdir;
    device_file += "/conf/gen/device_info.txt";

    ifs.open(device_file, std::ifstream::in);
    if (!ifs.is_open()) {
        return false;
    }

    while (getline(ifs, line)) {
        found_pos = line.find(LEARN_UIO_DEV_NAME);
        if (found_pos != string::npos) {
            found = true;
            break;
        }
    }

    ifs.close();
    return found; 
}

static bool
uio_device_ready (void)
{
    pds_state *state = &api::g_pds_state;

    switch (state->platform_type()) {
    case platform_type_t::PLATFORM_TYPE_HW:
    case platform_type_t::PLATFORM_TYPE_HAPS:
        return uio_device_ready_hw();
        break;
    case platform_type_t::PLATFORM_TYPE_SIM:
        return uio_device_ready_sim();
        break;
    default:
        PDS_TRACE_ERR("Invalid platform type[%s], asserting!!",
                      PLATFORM_TYPE_str(state->platform_type()));
        SDK_ASSERT(0);
        break;
    }
    return false;
}

#endif  // __LEARN_UIO_HPP__
