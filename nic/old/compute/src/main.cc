// TODO -
// cleanup these includes, these are included only to make sure code compiles
// make sure each header file brings all its dependencies by itself (i.e.,
// code should compile by just including that header file alone and not
// explicitly include other headers that it depends on
#include <log.hpp>
#include <init.hpp>
#include <action.hpp>
#include <eth.hpp>
#include <l2.hpp>
#include <ip.hpp>
#include <l3.hpp>
#include <if.hpp>
#include <sfw.hpp>
#include <flow.hpp>
#include <pkt.hpp>

int
main (int argc, char *argv[])
{
    // TODO: do all shared memory mapping etc.

    // initialize the infra layer
    svc_init();

    // do service specific initialization, if any
    if_init();
    l2_init();
    l3_init();
    l4_init();
    l7_init();

    while (1) {
        // pick next packet and process ...
    }

    return 0;
}
