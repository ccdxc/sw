#include <arpa/inet.h>
#include <inttypes.h>

#include "gen/proto/alerts.pb.h"
#include "lib/operd/operd.hpp"

extern "C" {

void
handler(sdk::operd::log_ptr entry)
{
    operd::Alert alert;
    bool result = alert.ParseFromArray(entry->data(), entry->data_length());
    assert(result == true);

    printf("%s %s %s %s\n", alert.name().c_str(), alert.category().c_str(),
           alert.description().c_str(), alert.message().c_str());
}

} // extern "C"
