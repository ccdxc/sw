// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package testenv

// This package contains flags to indicate to various components whether
// they are being run in the context of a test or not.
// These flags are per-component and default to "production mode". The
// init() function determines whether to switch to "test mode" or not.
// Right now the decision is based on an environment variable named "VENICE_DEV",
// which is set in dev and CI VMs, but not in the containers in which Venice
// components are packaged.
// Whenever a component requires a different behavior for "test mode" vs.
// "production mode", it should check the flags in this package.
