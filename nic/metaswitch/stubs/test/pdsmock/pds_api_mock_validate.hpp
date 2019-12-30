//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __PDS_MS_PDS_MOCK_VALIDATE__
#define __PDS_MS_PDS_MOCK_VALIDATE__ 

#include "nic/metaswitch/stubs/test/pdsmock/pds_api_mock.hpp"

namespace pds_ms_test {
bool
pds_tep_mock_validate(const spec_t&  expected_pds,
                      const spec_t&  rcvd_pds);
bool
pds_nhgroup_mock_validate(const spec_t&  expected_pds,
                          const spec_t&  rcvd_pds);
bool
pds_if_mock_validate(const spec_t&  expected_pds,
                     const spec_t&  rcvd_pds);
bool
pds_subnet_mock_validate(const spec_t&  expected_pds,
                         const spec_t&  rcvd_pds);
bool
pds_vpc_mock_validate(const spec_t&  expected_pds,
                         const spec_t&  rcvd_pds);

} // End namespace pds_ms_test
#endif
