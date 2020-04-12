//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <getopt.h>
#include <string>
#include <iostream>

#include "alerts_gen.hpp"

using namespace std;
using namespace test::alerts;

// max alerts per second supported
static constexpr int g_max_rps = 10000;

void print_usage(char* name)
{
    cerr << "Usage: " << name << endl         
         << "\t -r rate per second [default: 1]\n"
         << "\t -n number of alerts [default: 1]\n"
         << "\t -t type of alert [default: -1 (random type)] \n"
         << "\t -p prefix for alert messages [default: 'TEST_ALERT_GEN']\n"
         << "\t -h help \n"
         << endl;
}

int main(int argc, char** argv)
{
    int opt;
    int rate_per_second = 1;
    int total_alerts = 1;
    int alert_type = -1;
    string alert_msg_pfx = "TEST_ALERT_GEN";
    alerts_generator *generator;

    while ((opt = getopt(argc, argv, "r:n:t:p:h")) != -1) {
        switch (opt) {
        case 'r':
            rate_per_second = atoi(optarg);
            if (rate_per_second > g_max_rps) {
                cerr << "option -"
                     << optopt
                     << " requires value in between 1 and "
                     << g_max_rps
                     << endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 'n':
            total_alerts = atoi(optarg);
            break;
        case 't':
            alert_type = atoi(optarg);
            if ((alert_type < -1) ||
                (alert_type >= alerts_generator::k_max_alert_types)) {
                cerr << "option -"
                     << optopt
                     << " requires value in between -1 and "
                     << alerts_generator::k_max_alert_types - 1
                     << endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            alert_msg_pfx = string(optarg);
            break;
        case 'h':
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        case ':':
            cerr << "option -" << optopt << " requires an argument" << endl;
            print_usage(argv[0]);
            // fall through
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (total_alerts < rate_per_second) {
        cerr << "[-n total alerts] should be >= rate per second" << endl;
        exit(EXIT_FAILURE);
    }

    generator = alerts_generator::factory(rate_per_second, total_alerts,
                                          alert_type, alert_msg_pfx);
    if (generator == NULL) {
        cerr << "Failed to create alerts generator object" << endl;
        exit(EXIT_FAILURE);
    }
    generator->show_config();
    generator->generate_alerts();
    alerts_generator::destroy(generator);

    exit(EXIT_SUCCESS);
}
