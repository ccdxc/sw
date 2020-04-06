/*
 * Copyright (c) 2019, Pensando Systems Inc.
*/

#include "asic/pd/pd.hpp"
#include "sysmon_internal.hpp"
#include "platform/sensor/sensor.hpp"
#include "platform/drivers/xcvr_qsfp.hpp"

using namespace sdk::asic::pd;
using namespace sdk::platform::sensor;

static pd_adjust_perf_index_t perf_id = PD_PERF_ID0;
int startingfrequency_1100 = 0;

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
            SDK_HMON_TRACE_INFO("Increased the frequency.");
        } else {
            if (perf_id != PD_PERF_ID4) {
                SDK_HMON_TRACE_ERR("Unable to change the frequency failed, "
                                   "perf_id is %u", perf_id);
            }
        }
    } else if (hbmtemperature >= HBM_TEMP_UPPER_LIMIT) {
        status = asic_pd_adjust_perf(chip_id, inst_id, perf_id, PD_PERF_DOWN);
        if (status == PD_PERF_SUCCESS) {
            SDK_HMON_TRACE_INFO("Decreased the frequency.");
        } else {
            if (perf_id != PD_PERF_ID0) {
                SDK_HMON_TRACE_ERR("Unable to change the frequency failed, "
                                   "perf_id is %u", perf_id);
            }
        }
    } else {
        return;
    }
}

void
checktemperature(void)
{
    int ret;
    int chip_id = 0;
    int inst_id = 0;
    static int max_die_temp;
    static int max_local_temp;
    static int max_hbm_temp;
    static sysmond_hbm_threshold_event_t prev_hbmtemp_event;
    sysmond_hbm_threshold_event_t hbmtemp_event;
    sdk::platform::sensor::system_temperature_t temperature;

    ret = read_temperatures(&temperature);
    if (!ret) {
        temperature.dietemp /= 1000;
        if (max_die_temp < temperature.dietemp) {
            SDK_HMON_TRACE_INFO("%s is : %uC",
                                "Die temperature", temperature.dietemp);
            max_die_temp = temperature.dietemp;
        }

        temperature.localtemp /= 1000;
        if (max_local_temp < temperature.localtemp) {
            SDK_HMON_TRACE_INFO("%s is : %uC",
                                "Local temperature", temperature.localtemp);
            max_local_temp = temperature.localtemp;
        }

        if (max_hbm_temp < temperature.hbmtemp) {
            SDK_HMON_TRACE_INFO("HBM temperature is : %uC",
                                temperature.hbmtemp);
            max_hbm_temp = temperature.hbmtemp;
        }

        //TODO walk the ports once notification works correctly
        sdk::platform::read_qsfp_temperature(0, &temperature.xcvrtemp[0]);
        sdk::platform::read_qsfp_temperature(1, &temperature.xcvrtemp[1]);

        if (startingfrequency_1100 == 1) {
            changefrequency(temperature.hbmtemp);
        }
        if ((temperature.hbmtemp >=
                 g_sysmon_cfg.catalog->hbmtemperature_threshold()) &&
            (prev_hbmtemp_event != SYSMOND_HBM_TEMP_ABOVE_THRESHOLD)) {
            SDK_HMON_TRACE_ERR(
                "HBM temperature is : %uC ***, threshold is %u",
                temperature.hbmtemp,
                g_sysmon_cfg.catalog->hbmtemperature_threshold());
            SDK_HMON_TRACE_INFO(
                "HBM temperature is : %uC ***, threshold is %u",
                temperature.hbmtemp,
                g_sysmon_cfg.catalog->hbmtemperature_threshold());
            hbmtemp_event = SYSMOND_HBM_TEMP_ABOVE_THRESHOLD;
            prev_hbmtemp_event = SYSMOND_HBM_TEMP_ABOVE_THRESHOLD;
        } else if ((prev_hbmtemp_event == SYSMOND_HBM_TEMP_ABOVE_THRESHOLD) &&
                   (temperature.hbmtemp <
                        g_sysmon_cfg.catalog->hbmtemperature_threshold())) {
            hbmtemp_event = SYSMOND_HBM_TEMP_BELOW_THRESHOLD;
            prev_hbmtemp_event = SYSMOND_HBM_TEMP_BELOW_THRESHOLD;
        } else {
            hbmtemp_event = SYSMOND_HBM_TEMP_NONE;
        }
        if (g_sysmon_cfg.temp_event_cb) {
            g_sysmon_cfg.temp_event_cb(&temperature, hbmtemp_event);
        }
    } else {
        SDK_HMON_TRACE_ERR("Reading temperature failed");
    }

    return;
}

// MONFUNC(checktemperature);

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
        SDK_HMON_TRACE_ERR("%s", ex.what());
        return -1;
    }

    if (input.empty()) {
        return -1;
    }

    if (input.get_optional<std::string>(FREQUENCY_KEY)) {
        try {
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
            status = asic_pd_adjust_perf(chip_id, inst_id, perf_id,
                                         PD_PERF_SET);
        } catch (std::exception const &ex) {
            SDK_HMON_TRACE_ERR("%s", ex.what());
            return -1;
        }
    }

    if (status == PD_PERF_SUCCESS) {
        return 0;
    }
    return -1;
}
