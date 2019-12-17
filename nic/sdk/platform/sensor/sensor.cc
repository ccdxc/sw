/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include "sensor.hpp"

namespace sdk {
namespace platform {
namespace sensor {

static int
read_file(const char *filename, int *variable)
{
    char buf[32] = {0, };
    int fd;
    ssize_t len;
    uint32_t var;

    fd = open(filename, O_RDONLY);
    if (fd < 1) {
        return -ENOENT;
    }
    len = read(fd, buf, sizeof(buf) - 1);
    if (len < 0) {
        close(fd);
        return -EIO;
    }
    buf[len] = 0;
    sscanf(buf, "%d", &var);
    *variable = var;
    close(fd);

    return 0;
}

int read_local_temperature(int *localtemp)
{
    if (localtemp == NULL) {
        return -EINVAL;
    }
    return read_file(LOCAL_TEMP_FILE, localtemp);
}

static
int read_die_temperature(int *dietemp)
{
    if (dietemp == NULL) {
        return -EINVAL;
    }
    return read_file(DIE_TEMP_FILE, dietemp);
}

static int read_pin(int *pin)
{
    if (pin == NULL) {
        return -EINVAL;
    }
    return read_file(PIN_INPUT_FILE, pin);
}

static int read_pout1(int *pout1)
{
    if (pout1 == NULL) {
        return -EINVAL;
    }
    return read_file(POUT1_INPUT_FILE, pout1);
}

static int read_pout2(int *pout2)
{
    if (pout2 == NULL) {
        return -EINVAL;
    }
    return read_file(POUT2_INPUT_FILE, pout2);
}

static int read_vin(int *vin)
{
    if (vin == NULL) {
        return -EINVAL;
    }
    return read_file(VIN_INPUT_FILE, vin);
}

static int read_vout1(int *vout1)
{
    if (vout1 == NULL) {
        return -EINVAL;
    }
    return read_file(VOUT1_INPUT_FILE, vout1);
}

static int read_vout2(int *vout2)
{
    if (vout2 == NULL) {
        return -EINVAL;
    }
    return read_file(VOUT2_INPUT_FILE, vout2);
}
/**
    Read temperature function provides an API to read
    all the temperatures and fills in the structure.
    Requires sdkcapri_csrint to be linked
*/
int read_temperatures(system_temperature_t *temperature)
{
    if (temperature == NULL) {
        return -EINVAL;
    }
    if (read_local_temperature(&temperature->localtemp) == 0 &&
        read_die_temperature(&temperature->dietemp) == 0)
    {
        temperature->dietemp = temperature->dietemp - DIE_TEMP_STANDARD_DEVIATION;
        temperature->hbmtemp = cap_nwl_sbus_get_1500_temperature();
        temperature->qsfp1temp = 40;
        temperature->qsfp2temp = 40;
        temperature->qsfp1warningtemp = 70;
        temperature->qsfp2warningtemp = 70;
        temperature->qsfp1alarmtemp = 75;
        temperature->qsfp2alarmtemp = 75;
        return 0;
    }
    return -EINVAL;
}

/**
    Read power function provides an API to read
    all the power and fills in the structure.
*/
int read_powers(system_power_t *power)
{
    if (power == NULL) {
        return -EINVAL;
    }
    if ((read_pin(&power->pin) == 0) &&
        (read_pout1(&power->pout1) == 0) &&
        (read_pout2(&power->pout2) == 0))
    {
        return 0;
    }
    return -EINVAL;
}

/**
    Read voltage function provides an API to read
    all the volatges and fills in the structure.
*/
int read_voltages(system_voltage_t *voltage)
{
    if (voltage == NULL) {
        return -EINVAL;
    }
    if (read_vin(&voltage->vin) == 0 &&
        read_vout1(&voltage->vout1) == 0 &&
        read_vout2(&voltage->vout2) == 0)
    {
        return 0;
    }
    return -EINVAL;
}
} // namespace sensor
} // namespace platform
} // namespace sdk
