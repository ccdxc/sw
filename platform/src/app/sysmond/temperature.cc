/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "asic/pd/pd.hpp"

using namespace sdk::asic::pd;
using namespace sdk::platform::sensor;
extern shared_ptr<SysmondService> svc;

static delphi::objects::asictemperaturemetrics_t    asictemp;
static pd_adjust_perf_index_t perf_id = PD_PERF_ID0;
int startingfrequency_1100 = 0;
sdk::lib::catalog  *catalog = NULL;

#define FREQUENCY_FILE "/sysconfig/config0/frequency.json"
#define FREQUENCY_KEY "frequency"

#define HBM_TEMP_LOWER_LIMIT 85
#define HBM_TEMP_UPPER_LIMIT 95

static void
changefrequency(uint64_t hbmtemperature) {

    pd_adjust_perf_status_t status;
    int chip_id = 0;
    int inst_id = 0;

    if (hbmtemperature <= HBM_TEMP_LOWER_LIMIT) {
        status = asic_pd_adjust_perf(chip_id, inst_id, perf_id, PD_PERF_UP);
        if (status == PD_PERF_SUCCESS) {
            TRACE_INFO(GetLogger(), "Increased the frequency.");
        } else {
            if (perf_id != PD_PERF_ID4) {
                TRACE_ERR(GetLogger(), "Unable to change the frequency failed, perf_id is {}", perf_id);
            }
        }
    } else if (hbmtemperature >= HBM_TEMP_UPPER_LIMIT) {
        status = asic_pd_adjust_perf(chip_id, inst_id, perf_id, PD_PERF_DOWN);
        if (status == PD_PERF_SUCCESS) {
            TRACE_INFO(GetLogger(), "Decreased the frequency.");
        } else {
            if (perf_id != PD_PERF_ID0) {
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
    uint64_t key = 0;
    int ret;
    int chip_id = 0;
    int inst_id = 0;
    static int max_die_temp;
    static int max_local_temp;
    static int max_hbm_temp;
    sdk::platform::sensor::system_temperature_t temperature;

    ret = read_temperatures(&temperature);
    if (!ret) {
        temperature.dietemp /= 1000;
        if (max_die_temp < temperature.dietemp) {
            TRACE_INFO(GetLogger(), "{:s} is : {:d}C",
                       "Die temperature", temperature.dietemp);
            max_die_temp = temperature.dietemp;
        }
        asictemp.die_temperature = temperature.dietemp;

        temperature.localtemp /= 1000;
        if (max_local_temp < temperature.localtemp) {
            TRACE_INFO(GetLogger(), "{:s} is : {:d}C",
                       "Local temperature", temperature.localtemp);
            max_local_temp = temperature.localtemp;
        }
        asictemp.local_temperature = temperature.localtemp;

        if (max_hbm_temp < temperature.hbmtemp) {
            TRACE_INFO(GetLogger(), "HBM temperature is : {:d}C", temperature.hbmtemp);
            max_hbm_temp = temperature.hbmtemp;
        }
        asictemp.hbm_temperature = temperature.hbmtemp;

        if (startingfrequency_1100 == 1) {
            changefrequency(asictemp.hbm_temperature);
        }
        if (asictemp.hbm_temperature >= catalog->hbmtemperature_threshold()) {
            TRACE_INFO(GetObflLogger(), "HBM temperature is : {:d}C *** and threshold is {:d} Throttling down",
                       asictemp.hbm_temperature, catalog->hbmtemperature_threshold());
            TRACE_INFO(GetLogger(), "HBM temperature is : {:d}C *** and threshold is {:d} Throttling down",
                       asictemp.hbm_temperature, catalog->hbmtemperature_threshold());
            TRACE_FLUSH(GetObflLogger());
            asic_pd_set_half_clock(chip_id, inst_id);
            svc->ChangeAsicFrequency();
        }
        //Publish Delphi object
        delphi::objects::AsicTemperatureMetrics::Publish(key, &asictemp);
    } else {
        TRACE_ERR(GetLogger(), "Reading temperature failed");
    }

    return;
}

MONFUNC(checktemperature);

int configurefrequency() {
    boost::property_tree::ptree input;
    pd_adjust_perf_status_t status = PD_PERF_FAILED;
    int chip_id = 0;
    int inst_id = 0;
    string frequency;

    try {
        boost::property_tree::read_json(FREQUENCY_FILE, input);
    }
    catch (std::exception const &ex) {
        TRACE_ERR(GetLogger(), "{}", ex.what());
        return -1;
    }

    if (input.empty()) {
        return -1;
    }

    if (input.get_optional<std::string>(FREQUENCY_KEY)) {
        frequency = input.get<std::string>(FREQUENCY_KEY);
        if (frequency.compare("833") == 0) {
            perf_id = PD_PERF_ID0;
        } else if (frequency.compare("900") == 0) {
            perf_id = PD_PERF_ID1;
        } else if (frequency.compare("957") == 0) {
            perf_id = PD_PERF_ID2;
        } else if (frequency.compare("1033") == 0) {
            perf_id = PD_PERF_ID3;
        } else if (frequency.compare("1100") == 0) {
            startingfrequency_1100 = 1;
            perf_id = PD_PERF_ID4;
        } else {
            return -1;
        }
        status = asic_pd_adjust_perf(chip_id, inst_id, perf_id, PD_PERF_SET);
    }

    if (status == PD_PERF_SUCCESS) {
        return 0;
    }
    return -1;
}
