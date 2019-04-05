/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <iostream>
#include <string>
#include "third-party/asic/capri/verif/apis/cap_platform_api.h"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/platform/sensor/sensor.hpp"

#define STANDARD_DEVIATION_PERCENTAGE 3

static void
usage(void)
{
    fprintf(stderr, "powerctl (-disp)\n");
    fprintf(stderr, "-disp: Will Display the VDD and ARM voltages\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    sdk::lib::catalog  *catalog      = NULL;
    int standard_deviation = 0;
    sdk::platform::sensor::system_voltage_t voltage;
    if (argc < 2) {
        usage();
    }

    catalog = sdk::lib::catalog::factory();

    if (!strcmp(argv[1], "-disp")) {
        if (sdk::platform::sensor::read_voltages(&voltage) == 0) {
            printf("vdd voltage is %dmV\n", voltage.vout1);
            printf("arm voltage is %dmV\n", voltage.vout2);
        }
    } else if (!strcmp(argv[1], "-set")) {
        int startup_vdd;
        startup_vdd = catalog->startup_vdd();

        if (sdk::platform::sensor::read_voltages(&voltage) == 0) {
            if(startup_vdd == 0) {
                // NOP for the card.
                printf("%dmV observed. No change required.\n", voltage.vout1);
            } else {
                standard_deviation = ((startup_vdd * STANDARD_DEVIATION_PERCENTAGE) / 100);
                printf("%dmV standard deviation\n", standard_deviation);
                printf("%dmV lower range\n", startup_vdd - standard_deviation);
                printf("%dmV upper range\n", startup_vdd + standard_deviation);
                if (voltage.vout1 >= startup_vdd - standard_deviation &&
                    voltage.vout1 <= startup_vdd + standard_deviation) {
                    // Already at the desired voltage.
                    printf("%dmV observed. No change required.\n", voltage.vout1);
                } else {
                    cap_set_margin_by_value("vdd", startup_vdd);
                    printf("%dmV observed. Changing to %dmV.\n", voltage.vout1, startup_vdd);
                }
            }
        } else {
            printf("Problem reading the current voltage.\n");
        }
    } else {
        usage();
    }

    return 0;
}
