//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <chrono>
#include <iostream>

#include "alerts_gen.hpp"

using namespace std;

namespace test {
namespace alerts {

alerts_generator *
alerts_generator::factory(int rps, int total, int type, string pfx) {
    return new alerts_generator(rps, total, type, pfx);
}

void
alerts_generator::destroy(alerts_generator *alerts_generator_inst) {
    delete alerts_generator_inst;
}

static inline string
get_alert_name (int alert_type)
{
    string alert_name;

    if (alert_type == -1) {
        alert_name = "random";
    } else {
        alert_name = operd::alerts::alerts[alert_type].name;
    }

    return alert_name;
}

void
alerts_generator::show_config(void) {
    cout << "Types of alert supported: " << k_max_alert_types << endl;
    cout << "Alerts rate per second: " << rps_ << endl;
    cout << "Total no. of alerts to be generated: " << total_alerts_ << endl;
    cout << "Alert msg prefix: " << alert_msg_pfx_ << endl;
    cout << "Alert type: " << get_alert_name(alert_type_) << endl;
}

void
alerts_generator::generate_alerts(void) {
    int num_alerts_raised = 0;
    int time_elapsed;
    using micros = chrono::microseconds;

    while (num_alerts_raised < total_alerts_) {
        auto start = chrono::steady_clock::now();
        for (int i = 0; i < rps_; ++i) {
            raise_alert_();
        }
        num_alerts_raised += rps_;
        auto finish = chrono::steady_clock::now();
        time_elapsed = chrono::duration_cast<micros>(finish - start).count();
        usleep(1000000 - time_elapsed);
    }
    cout << "No. of alerts generated " << total_alerts_ << endl;
}

int
alerts_generator::get_alert_type_(void) {
    if (alert_type_ == -1) {
        return (rand() % k_max_alert_types);
    }
    return alert_type_;
}

static inline string
get_alert_msg (void)
{
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    char timestring[30];

    strftime(timestring, 30, " alert %Y%m%d%H%M%S", localtime(&now));
    return string(timestring);
}

void
alerts_generator::raise_alert_(void) {
    operd_alerts_t  alert_type;
    string alert_msg;

    alert_type = (operd_alerts_t)get_alert_type_();
    alert_msg = alert_msg_pfx_ + get_alert_msg();
#if 0
    cout << "Raising alert " << get_alert_name(alert_type)
         << " with msg " <<  alert_msg << endl;
#endif
    recorder_->alert((operd_alerts_t)get_alert_type_(), alert_msg.c_str());
    return;
}

}    // namespace alerts
}    // namespace test
