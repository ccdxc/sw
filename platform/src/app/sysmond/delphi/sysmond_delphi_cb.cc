//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include "gen/proto/sysmond.delphi.hpp"
#include "platform/src/app/sysmond/logger.h"
#include "platform/src/app/sysmond/delphi/sysmond_delphi.hpp"
#include "nic/sdk/platform/sensor/sensor.hpp"
#include "platform/src/app/sysmond/sysmond_cb.hpp"

static shared_ptr<SysmondService> svc;
static delphi::objects::asicpowermetrics_t asicpower;
static delphi::objects::asictemperaturemetrics_t asictemp;
static delphi::objects::asicmemorymetrics_t asicmemory;
static delphi::SdkPtr g_sdk;

void
delphi_event_cb_init (void)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    pthread_t delphi_thread;

    // Register for sysmond metrics
    delphi::objects::AsicTemperatureMetrics::CreateTable();
    delphi::objects::AsicPowerMetrics::CreateTable();
    delphi::objects::AsicFrequencyMetrics::CreateTable();
    delphi::objects::AsicMemoryMetrics::CreateTable();

    // Register for asicerror metrics
    create_tables();

    svc = make_shared<SysmondService>(sdk, "Sysmond");
    svc->init();
    sdk->RegisterService(svc);
    sdk->Connect();
    g_sdk = sdk;
}

void
delphi_frequency_change_event_cb (uint32_t frequency)
{
    uint64_t key = 0;
    int chip_id = 0;
    int inst_id = 0;
    static delphi::objects::asicfrequencymetrics_t asicfrequency;

    TRACE_INFO(GetLogger(), "Core frequency of the system {}MHz", frequency);
    asicfrequency.frequency = frequency;
    delphi::objects::AsicFrequencyMetrics::Publish(key, &asicfrequency);
    return;
}

void
delphi_cattrip_event_cb (void)
{
    TRACE_ERR(GetLogger(), "Cattrip occurred");
    TRACE_FLUSH(GetLogger());
    return;
}

void
delphi_power_event_cb (sdk::platform::sensor::system_power_t *power)
{
    uint64_t key = 0;

    asicpower.pin = power->pin;
    asicpower.pout1 = power->pout1;
    asicpower.pout2 = power->pout2;

    //Publish Delphi object
    delphi::objects::AsicPowerMetrics::Publish(key, &asicpower);
    return;
}

void
delphi_temp_event_cb (sdk::platform::sensor::system_temperature_t *temperature)
{
    uint64_t key = 0;

    asictemp.hbm_temperature = temperature->hbmtemp;
    asictemp.local_temperature = temperature->localtemp;
    asictemp.die_temperature = temperature->dietemp;
    asictemp.qsfp_port1_temperature = temperature->xcvrtemp[0].temperature;
    asictemp.qsfp_port2_temperature = temperature->xcvrtemp[1].temperature;
    asictemp.qsfp_port1_warning_temperature = temperature->xcvrtemp[0].warning_temperature;
    asictemp.qsfp_port2_warning_temperature = temperature->xcvrtemp[1].warning_temperature;
    asictemp.qsfp_port1_alarm_temperature = temperature->xcvrtemp[0].alarm_temperature;
    asictemp.qsfp_port2_alarm_temperature = temperature->xcvrtemp[1].alarm_temperature;

    // Publish Delphi object
    delphi::objects::AsicTemperatureMetrics::Publish(key, &asictemp);
    return;
}

void
delphi_memory_event_cb (uint64_t total_mem, uint64_t available_mem,
                 uint64_t free_mem)
{
    uint64_t key = 0;

    asicmemory.totalmemory = total_mem;
    asicmemory.availablememory = available_mem;
    asicmemory.freememory = free_mem;

    //Publish Delphi object
    delphi::objects::AsicMemoryMetrics::Publish(key, &asicmemory);
    return;
}
