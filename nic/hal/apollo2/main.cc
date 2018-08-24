//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "nic/hal/apollo2/hal.hpp"

//------------------------------------------------------------------------------
// helper function to dump the usage of this HAL app
//------------------------------------------------------------------------------
static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json>\n", argv[0]);
}

//------------------------------------------------------------------------------
// entry point for this HAL app
//------------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    int     oc;
    char    *cfg_file = NULL;

    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            cfg_file = optarg;
            if (!cfg_file) {
                fprintf(stderr, "config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'h':
            print_usage(argv);
            exit(0);
            break;

        case ':':
            fprintf(stderr, "%s: option -%c requires an argument\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;

        case '?':
        default:
            fprintf(stderr, "%s: option -%c is invalid, quitting ...\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;
        }
    }
    hal::init(cfg_file);
    return 0;
}
