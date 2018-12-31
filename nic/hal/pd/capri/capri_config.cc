#include "nic/include/base.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"

#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

typedef struct addr_data_ {
    uint32_t   addr;
    uint32_t   data;
} addr_data_t;

hal_ret_t
capri_load_config (char *config_dir)
{
    hal_ret_t          ret = HAL_RET_OK;
    DIR                *dirp;
    struct dirent      *file;
    int                fd, rs;
    addr_data_t        buff[512];
    uint16_t           readsz, nelems;
    std::string        cfg_fname;

    dirp = opendir(config_dir);
    if (dirp == NULL) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", config_dir);
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }
    while ((file = readdir(dirp))) {
        if (!strcmp(file->d_name,".") || !strcmp(file->d_name, "..")) {
            continue;
        }

        HAL_TRACE_DEBUG("Processing config file {}", file->d_name);
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
            HAL_TRACE_ERR("Unable to close file: {}, err: {}",
                          file->d_name, rs);
            continue;
        }
    }

    closedir(dirp);

    return ret;
}

hal_ret_t
capri_verify_config (char *config_dir)
{
    hal_ret_t          ret = HAL_RET_OK;
    DIR                *dirp;
    struct dirent      *file;
    int                fd, rs;
    addr_data_t        buff[512];
    uint16_t           readsz, nelems;
    std::string        cfg_fname;
    uint32_t           data;

    dirp = opendir(config_dir);
    if (dirp == NULL) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", config_dir);
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }
    while ((file = readdir(dirp))) {
        if (!strcmp(file->d_name,".") || !strcmp(file->d_name, "..")) {
            continue;
        }

        HAL_TRACE_DEBUG("Processing config file {}", file->d_name);
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
                    HAL_TRACE_DEBUG("Reg config does not match addr {:#x} "
                                    "data {:#x} expected {:#x}",
                                    buff[i].addr, data, buff[i].data);
                }
            }
        }
        rs = close(fd);
        if (rs) {
            HAL_TRACE_ERR("Unable to close file: {}, err: {}",
                          file->d_name, rs);
            continue;
        }
    }

    closedir(dirp);

    return ret;
}
