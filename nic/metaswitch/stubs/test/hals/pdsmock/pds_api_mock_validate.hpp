//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDSA_PDS_MOCK_VALIDATE__
#define _PDSA_PDS_MOCK_VALIDATE__

#include "nic/metaswitch/stubs/test/hals/pdsmock/pds_api_mock.hpp"

namespace pdsa_test {
bool
pds_tep_mock_validate(const spec_t&  expected_pds,
                      const spec_t&  rcvd_pds);
bool
pds_nhgroup_mock_validate(const spec_t&  expected_pds,
                          const spec_t&  rcvd_pds);

} // End namespace pdsa_test
#endif
