//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all tep test cases
///
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// TEP test class
//----------------------------------------------------------------------------

class tep_test : public oci_test_base {
protected:
    tep_test() {}
    virtual ~tep_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// TEP test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// TEP test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP
/// @{

/// \brief Create a TEP
///
/// Detailed description
TEST_F(tep_test, tep_create) {}

/// \brief Delete a TEP
///
/// Detailed description
TEST_F(tep_test, tep_delete) {}

/// @}
