/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "nic/sdk/platform/capri/csrint/csr_init.hpp"

static delphi::objects::asictemperaturemetrics_t    asictemp;
static pen_adjust_perf_index perf_id = PEN_PERF_ID0;
int frequencyfromfile = 0;

#define FREQUENCY_FILE "/sysconfig/config0/frequency.json"
#define FREQUENCY_KEY "frequency"
#define HBM_TEMP_LOWER_LIMIT 85
#define HBM_TEMP_UPPER_LIMIT 95

static void
changefrequency(uint64_t hbmtemperature) {

    pen_adjust_perf_status status;
    int chip_id = 0;
    int inst_id = 0;

    if (hbmtemperature <= HBM_TEMP_LOWER_LIMIT) {
        status = cap_top_adjust_perf(chip_id, inst_id, perf_id, PEN_PERF_UP);
        if (status == PEN_PERF_SUCCESS) {
            TRACE_INFO(GetLogger(), "Increased the frequency.");
        } else {
            TRACE_ERR(GetLogger(), "Unable to change the frequency failed, perf_id is {}", perf_id);
        }
    } else if (hbmtemperature >= HBM_TEMP_UPPER_LIMIT) {
        status = cap_top_adjust_perf(chip_id, inst_id, perf_id, PEN_PERF_DOWN);
        if (status == PEN_PERF_SUCCESS) {
            TRACE_INFO(GetLogger(), "Decreased the frequency.");
        } else {
            if (perf_id != PEN_PERF_ID4) {
                TRACE_ERR(GetLogger(), "Unable to change the frequency failed, perf_id is {}", perf_id);
            }
        }
    } else {
        return;
    }
}

static void
checktemperature(void)
{
    uint8_t key = 0;
    uint32_t ret;
    sdk::platform::sensor::system_temperature_t temperature;

    ret = sdk::platform::sensor::read_temperatures(&temperature);
    if (!ret) {
        asictemp.die_temperature = temperature.dietemp / 1000;
        TRACE_INFO(GetLogger(), "{:s} is : {:d}C",
                   "Die temperature", asictemp.die_temperature);
        asictemp.local_temperature = temperature.localtemp / 1000;
        TRACE_INFO(GetLogger(), "{:s} is : {:d}C",
                   "Local temperature", asictemp.local_temperature);
        asictemp.hbm_temperature = temperature.hbmtemp;
        TRACE_INFO(GetLogger(), "HBM temperature is : {:d}C", asictemp.hbm_temperature);

        if (frequencyfromfile == 1) {
            changefrequency(asictemp.hbm_temperature);
        }

        //Publish Delphi object
        delphi::objects::AsicTemperatureMetrics::Publish(key, &asictemp);
    } else {
        TRACE_ERR(GetLogger(), "Reading temperature failed");
    }

    return;
}

MONFUNC(checktemperature);

int changestartingfrequencyfromfile() {
    boost::property_tree::ptree input;
    pen_adjust_perf_status status = PEN_PERF_FAILED;
    int chip_id = 0;
    int inst_id = 0;
    string frequency;

    try {
        boost::property_tree::read_json(FREQUENCY_FILE, input);
    }
    catch (std::exception const &ex) {
        cout << ex.what() << endl;
        return -1;
    }

    if (input.empty()) {
        return -1;
    }

    sdk::platform::capri::csr_init();
    if (input.get_optional<std::string>(FREQUENCY_KEY)) {
        frequency = input.get<std::string>(FREQUENCY_KEY);
        if (frequency.compare("833") == 0) {
            perf_id = PEN_PERF_ID0;
        } else if (frequency.compare("900") == 0) {
            perf_id = PEN_PERF_ID1;
        } else if (frequency.compare("957") == 0) {
            perf_id = PEN_PERF_ID2;
        } else if (frequency.compare("1033") == 0) {
            perf_id = PEN_PERF_ID3;
        } else if (frequency.compare("1100") == 0) {
            frequencyfromfile = 1;
            perf_id = PEN_PERF_ID4;
        } else {
            return -1;
        }
        status = cap_top_adjust_perf(chip_id, inst_id, perf_id, PEN_PERF_SET);
    }

    if (status == PEN_PERF_SUCCESS) {
        return 0;
    }
    return -1;
}
