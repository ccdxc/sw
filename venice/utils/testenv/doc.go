// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package testenv

// This package contains flags to indicate to various components whether
// they are being run in the context of a test or not.
// These flags are per-component and default to "production mode". Package
// "testutils", which is included by all tests but not by any component,
// has an init() function that switches these flags to "test mode".
// Whenever a component requires a different behavior for "test mode" vs.
// "production mode", it should check the flags in this package.
