/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include "sensor.hpp"

namespace sdk {
namespace platform {
namespace sensor {

static int
read_file(const char *filename, uint64_t *variable)
{
    char buf[32] = {0, };
    int fd;
    ssize_t len;
    uint32_t var;

    fd = open(filename, O_RDONLY);
    if (fd < 1) {
        return ENOENT;
    }
    len = read(fd, buf, sizeof(buf) - 1);
    if (len < 0) {
        close(fd);
        return EIO;
    }
    buf[len] = 0;
    sscanf(buf, "%d", &var);
    *variable = var;
    close(fd);

    return 0;
}

int read_die_temperature(uint64_t *dietemp)
{
    if (dietemp == NULL) {
        return EINVAL;
    }
    return read_file(DIE_TEMP_FILE, dietemp);
}

int read_local_temperature(uint64_t *localtemp)
{
    if (localtemp == NULL) {
        return EINVAL;
    }
    return read_file(LOCAL_TEMP_FILE, localtemp);
}

int read_pin(uint64_t *pin)
{
    if (pin == NULL) {
        return EINVAL;
    }
    return read_file(PIN_INPUT_FILE, pin);
}

int read_pout1(uint64_t *pout1)
{
    if (pout1 == NULL) {
        return EINVAL;
    }
    return read_file(POUT1_INPUT_FILE, pout1);
}

int read_pout2(uint64_t *pout2)
{
    if (pout2 == NULL) {
        return EINVAL;
    }
    return read_file(POUT2_INPUT_FILE, pout2);
}

} // namespace sensor
} // namespace platform
} // namespace sdk
