# Testcases for route matching in route tables

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
        tunnels, nexthops, security groups and policies are configured.

        Following route tables are configured.
        #1 Overlap
            The route prefixes are overlapping.
            a) Route table with left overlap routes
               The left most prefixes in the interval tree are overlapping.
            b) Route table with left overlap routes and a default route
            c) Route table with multiple left overlap routes
            d) Route table with multiple left overlap routes and a default
               route.
            e) Route table with right overlap routes.
               The right most prefixes in the interval tree are overlapping.
            f) Route table with right overlap routes and a default route.
            g) Route table with multiple right overlap routes.
            h) Route table with multiple right overlap routes and a default
               route.

        #2 Adjacent routes
            The prefixes in the routes are consecutive.
            a) Route table with consecutive prefixes as routes
            b) Route table with consecutive prefixes as routes and a default
               route.
            c) Route table with maximum number of consecutive prefixes as
               routes.
            d) Route table with maximum number of consecutive prefixes as
               routes and a default route.

        #3 Full overlap routes:
            The routes have completely overlapping prefixes.
            a) Route table with fully overlapping routes.
            b) Route table with fully overlapping routes and a default route.
            c) Route table with maximum number of fully overlapping routes.
            c) Route table with maximum number of fully overlapping routes and
               a default route.

        #4 Disjoint:
            The routes are completely disjoint.
            a) Route table with completely disjoint routes.
            b) Route table with completely disjoint routes and a default route.

        #5 Route table with only a default route.

        #6 Route table with only host routes.

        #7 Route table with routes having black hole nexthop.

Testcases based on the type of routes in the route table
## Only default route
    Setup:
        A subnet with route table containing only a default route is selected.

    Trigger:
        Host to internet gateway packet with random destination address is
        sent to host interface.

    Expectation:
        Nexthop picked from default route is used for generating the VXLAN
        packet on uplink ports.

## Default route with maximum number of routes
    Setup:
        A subnet with route table containing maximum number of routes with
        a default route is selected.

    Trigger:
        Host to internet gateway packet with random destination address is
        sent to host interface.

    Expectation:
        Nexthop picked from matching route with highest priority is used for
        generating the VXLAN packet on uplink ports.

## Only host route
    Setup:
        A subnet with route table containing only host routes is selected.

    Trigger:
        Host to internet gateway packet with random destination address from
        any of the routes is sent to host interface.

    Expectation:
        Nexthop picked up from the route with highest priority is used for
        generating the VXLAN packet on uplink ports.

## No routes
    Setup:
        A subnet with route table containing no routes is selected.

    Trigger:
        Host to internet gateway packet with random destination address is
        sent to host interface.

    Expectation:
        No packet is seen on uplink ports.

## Route with blackhole nexthop
    Setup:
        A subnet with route table containing a route with blackhole nexthop
        is selected.

    Trigger:
        Host to internet gateway packet with random destination address is
        sent to host interface.

    Expectation:
        No packet is seen on uplink ports.

The following testcases are run for all the route table types defined in
'Initial setup' section.

## Random destination address
    Trigger:
        A Host to internet gateway packet is sent to host interface by choosing
        a random destination address,
            1) From the routetable as configured initially
            2) After updating route IP addresses and nexthops
            3) After rolling back the attributes of routetable to old values

    Expectation:
        Nexthop picked up from the route with highest priority is used for
        generating the VXLAN packet on uplink ports.

## First value as destination address:
    Trigger:
        A Host to internet gateway packet is sent to host interface by choosing
        first value from the interval tree as the destination address,
            1) From the routetable as configured initially
            2) After updating route IP addresses and nexthops
            3) After rolling back the attributes of routetable to old values

    Expectation:
        Nexthop picked up from the route with highest priority is used for
        generating the VXLAN packet on uplink ports.

## Last value as destination address:
    Trigger:
        A Host to internet gateway packet is sent to host interface by choosing
        last value from the interval tree as the destination address,
            1) From the routetable as configured initially
            2) After updating route IP addresses and nexthops
            3) After rolling back the attributes of routetable to old values

    Expectation:
        Nexthop picked up from the route with highest priority is used for
        generating the VXLAN packet on uplink ports.
