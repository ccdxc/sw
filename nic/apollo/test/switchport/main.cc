//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all switchport test cases
///
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// SwitchPort test class
//----------------------------------------------------------------------------

class switchport_test : public oci_test_base {
protected:
    switchport_test() {}
    virtual ~switchport_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// SwitchPort test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// SwitchPort test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SwitchPort
/// @{

/// \brief Create a SwitchPort
///
/// Detailed description
TEST_F(switchport_test, switchport_create) {}

/// \brief Delete a SwitchPort
///
/// Detailed description
TEST_F(switchport_test, switchport_delete) {}

/// @}
