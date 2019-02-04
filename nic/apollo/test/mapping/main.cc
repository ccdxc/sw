//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all mapping test cases
///
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Mapping test class
//----------------------------------------------------------------------------

class mapping_test : public oci_test_base {
protected:
    mapping_test() {}
    virtual ~mapping_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// Mapping test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Mapping
/// @{

/// \brief Create a Mapping
///
/// Detailed description
TEST_F(mapping_test, mapping_create) {}

/// \brief Delete a Mapping
///
/// Detailed description
TEST_F(mapping_test, mapping_delete) {}

/// @}
