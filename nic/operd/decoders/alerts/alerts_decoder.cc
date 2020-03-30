// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "nic/sdk/lib/operd/decoder.h"
#include "nic/operd/alerts/alert_defs.h"

static size_t
alerts_decoder (uint8_t encoder, const char *data, size_t data_length,
                char *output, size_t output_size)
{
    int alert_id = *(int *)data;
    const char *message = data + sizeof(int);
    pds::Alert alert;
    alert_t prototype = operd::alerts::alerts[alert_id];
    size_t len;

    alert.set_name(prototype.name);
    alert.set_category(prototype.category);
    alert.set_description(prototype.description);
    alert.set_message(message);
    if (strcmp(prototype.severity, "DEBUG") == 0) {
        alert.set_severity(pds::DEBUG);
    } else if (strcmp(prototype.severity, "INFO") == 0) {
        alert.set_severity(pds::INFO);
    } else if (strcmp(prototype.severity, "WARN") == 0) {
        alert.set_severity(pds::WARN);
    } else if (strcmp(prototype.severity, "CRITICAL") == 0){
        alert.set_severity(pds::CRITICAL);
    }
    
    len = alert.ByteSizeLong();
    assert(len <= output_size);
    bool result = alert.SerializeToArray(output, output_size);
    assert(result == true);

    return len;
}

extern "C" void
decoder_lib_init(register_decoder_fn register_decoder)
{
    register_decoder(OPERD_DECODER_ALERTS, alerts_decoder);
}
