//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all subnet test cases
///
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Subnet test class
//----------------------------------------------------------------------------

class subnet_test : public oci_test_base {
protected:
    subnet_test() {}
    virtual ~subnet_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// Subnet test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Subnet test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Subnet
/// @{

/// \brief Create a Subnet
///
/// Detailed description
TEST_F(subnet_test, subnet_create) {}

/// \brief Delete a Subnet
///
/// Detailed description
TEST_F(subnet_test, subnet_delete) {}

/// @}
