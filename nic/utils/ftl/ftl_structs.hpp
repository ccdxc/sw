//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_STRUCTS_HPP__
#define __FTL_STRUCTS_HPP__

#include <arpa/inet.h>
#ifdef APOLLO
#include "ftlv6_structs_apollo.hpp"
#include "ftlv4_structs_apollo.hpp"
#elif defined(ARTEMIS)
#include "ftlv6_structs_artemis.hpp"
#include "ftlv4_structs_artemis.hpp"
#elif defined(APULU)
#include "ftlv6_structs_apulu.hpp"
#include "ftlv4_structs_apulu.hpp"
#elif IRIS
#include "ftlv6_structs_iris.hpp"
#include "ftlv4_structs_iris.hpp"
#elif defined(POSEIDON)
#include "ftlv6_structs_poseidon.hpp"
#include "ftlv4_structs_poseidon.hpp"
#else
#error FTL is not suppored on IRIS pipeline (Yet!!).
#endif

#endif // __FTL_STRUCTS_HPP__
