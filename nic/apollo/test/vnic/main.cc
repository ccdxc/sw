//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all vnic test cases
///
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// VNIC test class
//----------------------------------------------------------------------------

class vnic_test : public oci_test_base {
protected:
    vnic_test() {}
    virtual ~vnic_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// VNIC test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

/// \brief Create a VNIC
///
/// Create a VNIC and test the following
///
/// Configure both ipv4 and ipv6 addresses and run the packet test
///
/// Configure min to max of vlan ids and verify with packet
///
/// Configure varying range of mpls slots and verify with packet
///
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_create) {}

/// \brief Create many VNICs
///
/// Create max number of VNICs and test the following
///
/// Configure varying range of vlan tags & mpls slots and verify with packet
///
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_many_create) {}

/// \brief Get VNIC
///
/// Configure the vnic and make-sure the parameters are configured properly
/// by getting the configured values back
TEST_F(vnic_test, vnic_get) {}

/// \brief VNIC source and destination check
///
/// Configure skip_src_dst_check to true/false and make sure the config by
/// sending both valid and invalid IP addresses.
///
/// Get the stats and makesure the drop count/reason are as expected
TEST_F(vnic_test, vnic_src_dst_check) {}

#if 0
/// \brief Delete a VNIC
///
/// Detailed description
TEST_F(vnic_test, vnic_delete) {}
#endif
/// @}
