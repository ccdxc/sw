#include <arpa/inet.h>
#include <inttypes.h>

#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "lib/operd/operd.hpp"

extern "C" {

void
handler(sdk::operd::log_ptr entry)
{
    pds::Alert alert;
    pds::FlowLog flow;

    bool result = alert.ParseFromArray(entry->data(), entry->data_length());
    assert(result == true);

    printf("%s %s %s %s", alert.name().c_str(), alert.category().c_str(),
           alert.description().c_str(), alert.message().c_str());
}

} // extern "C"
