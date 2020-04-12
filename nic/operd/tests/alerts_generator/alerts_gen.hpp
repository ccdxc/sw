//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __ALERTS_GEN_HPP__
#define __ALERTS_GEN_HPP__

#include "nic/operd/alerts/alerts.hpp"
#include "gen/alerts/alert_defs.h"

using namespace std;
using namespace operd::alerts;

namespace test {
namespace alerts {

class alerts_generator {
public:

    /// \brief factory method to allocate & initialize alerts_generator instance
    /// \return new instance of alerts_generator or NULL, in case of error
    static alerts_generator *factory(int rps=1, int total=1,
                                     int type=-1, string pfx="GEN");

    /// \brief free memory allocated to alerts_generator instance
    /// \param[in] alerts_generator pointer to alerts_generator instance
    static void destroy(alerts_generator *alerts_generator);

    /// \brief generates alerts
    void generate_alerts(void);

    /// \brief displays the configuration
    void show_config(void);

private:
    /// \brief parameterized constructor
    alerts_generator(int rps, int total, int type, string pfx) {
        rps_ = rps;
        total_alerts_ = total;
        alert_type_ = type;
        alert_msg_pfx_ = pfx;
        recorder_ = alert_recorder::get();
    }

    /// \brief destructor
    ~alerts_generator() {}

    /// \brief returns the alert type to be generated
    /// \return alert type if configured else random supported alert type
    int get_alert_type_(void);

    /// \brief raise single alert based on config
    void raise_alert_(void);

public:
    static constexpr int k_max_alert_types =
        (sizeof(operd::alerts::alerts) / sizeof(alert_t));

private:
    int rps_;                        ///< genaration rate per second
    int total_alerts_;               ///< number of alerts to be generated
    int alert_type_;                 ///< type of alert to be generated
    string alert_msg_pfx_;           ///< string to be prefixed with alert msg
    alert_recorder_ptr recorder_;    ///< operd alert recorder

};

}    // namespace alerts
}    // namespace test
#endif   // __ALERTS_GEN_HPP__