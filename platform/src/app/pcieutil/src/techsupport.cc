/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "nic/sdk/platform/misc/include/misc.h"
#include "cmd.h"

static const char *cmdlist[] = {
    "pcieutil port",
    "pcieutil portstats",
    "pcieutil stats",
    "pcieutil counters",
    "pcieutil dev",
    "pcieutil devintr",
    "pcieutil bar",
    "pcieutil pmt",
    "pcieutil prt",
    "pcieutil hdrt",
    "pcieutil dberr",
    "pcieutil cfg -xxxx",
    "pcieutil vpd",
    "pcieutil portaer",
    "pcieutil portcfg -xxxx",
    "pcieutil serdesfw",
    "pcieutil healthdetails",
    // Note: these might trigger ecc errors if hw hasn't used all entries.
    // Leave them out for now.  (Could init these tables then safe to dump.)
    //"pcieutil aximst",
    //"pcieutil itr_rwhdr",
    NULL
};

static void
techsupport(int argc, char *argv[])
{
    for (const char **cmd = cmdlist; *cmd; cmd++) {
        // make a local copy for cmd_runstr to edit
        char cmdstr[80];
        strncpy0(cmdstr, *cmd, sizeof(cmdstr));

        printf("================\n"
               "%s\n"
               "----------------\n", cmdstr);
        cmd_runstr(cmdstr);
    }
}
CMDFUNC(techsupport,
"collect pcie techsupport",
"techsupport\n");
