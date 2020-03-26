// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include "asic/asic.hpp"
#include "asic/rw/asicrw.hpp"

namespace sdk  {
namespace asic {

typedef struct addr_data_ {
    uint32_t   addr;
    uint32_t   data;
} addr_data_t;


sdk_ret_t
asic_load_config (char *config_dir)
{
    sdk_ret_t          ret = SDK_RET_OK;
    DIR                *dirp;
    struct dirent      *file;
    int                fd, rs;
    addr_data_t        buff[512];
    uint16_t           readsz, nelems;
    std::string        cfg_fname;

    dirp = opendir(config_dir);
    if (dirp == NULL) {
        SDK_TRACE_ERR("%s not_present/no_read_permissions", config_dir);
        SDK_ASSERT_RETURN(0, SDK_RET_ERR);
    }
    while ((file = readdir(dirp))) {
        if (!strcmp(file->d_name,".") || !strcmp(file->d_name, "..")) {
            continue;
        }

        SDK_TRACE_DEBUG("Processing config file %s", file->d_name);
        cfg_fname = std::string(config_dir) + "/" + std::string(file->d_name);
        fd = open(cfg_fname.c_str(), O_RDONLY);
        if (fd == -1) {
            continue;
        }

        while ((readsz = read(fd, buff, sizeof(buff))) > 0) {
            nelems = readsz / sizeof(addr_data_t);
            for (int i = 0; i < nelems; i++) {
                sdk::asic::asic_reg_write(buff[i].addr, &buff[i].data, 1,
                                          ASIC_WRITE_MODE_WRITE_THRU);
            }
        }
        rs = close(fd);
        if (rs) {
            SDK_TRACE_ERR("Unable to close file: %s, err: %d",
                          file->d_name, rs);
            continue;
        }
    }

    closedir(dirp);

    return ret;
}

sdk_ret_t
asic_verify_config (char *config_dir)
{
    sdk_ret_t          ret = SDK_RET_OK;
    DIR                *dirp;
    struct dirent      *file;
    int                fd, rs;
    addr_data_t        buff[512];
    uint16_t           readsz, nelems;
    std::string        cfg_fname;
    uint32_t           data;

    dirp = opendir(config_dir);
    if (dirp == NULL) {
        SDK_TRACE_ERR("%s not_present/no_read_permissions", config_dir);
        SDK_ASSERT_RETURN(0, SDK_RET_ERR);
    }
    while ((file = readdir(dirp))) {
        if (!strcmp(file->d_name,".") || !strcmp(file->d_name, "..")) {
            continue;
        }

        SDK_TRACE_DEBUG("Processing config file %s", file->d_name);
        cfg_fname = std::string(config_dir) + "/" + std::string(file->d_name);
        fd = open(cfg_fname.c_str(), O_RDONLY);
        if (fd == -1) {
            continue;
        }

        while ((readsz = read(fd, buff, sizeof(buff))) > 0) {
            nelems = readsz / sizeof(addr_data_t);
            for (int i = 0; i < nelems; i++) {
                sdk::asic::asic_reg_read(buff[i].addr, &data, 1, true);
                if (data != buff[i].data) {
                    SDK_TRACE_DEBUG("Reg config does not match addr 0x%x"
                                    "data 0x%x expected 0x%x",
                                    buff[i].addr, data, buff[i].data);
                }
            }
        }
        rs = close(fd);
        if (rs) {
            SDK_TRACE_ERR("Unable to close file: %s, err: %d",
                          file->d_name, rs);
            continue;
        }
    }
    closedir(dirp);
    return ret;
}

}     // namespace asic 
}     // namespace sdk 
