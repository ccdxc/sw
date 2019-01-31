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
/// Detailed description
TEST_F(vnic_test, vnic_create) {}

/// \brief Delete a VNIC
///
/// Detailed description
TEST_F(vnic_test, vnic_delete) {}

/// @}
