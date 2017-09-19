// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFileBasedProviderInit(t *testing.T) {
	// make sure initialization bad cert file names fail
	_, err := NewFileBasedProvider("", "", "")
	Assert(t, (err != nil), "LocalFS TLS provider initialization with bad cert succeded while expecting it to fail")

	_, err = NewFileBasedProvider("test.crt", "", "")
	Assert(t, (err != nil), "LocalFS TLS provider initialization with bad cert succeded while expecting it to fail")

	_, err = NewFileBasedProvider("test.crt", "test.key", "testcerts/testCA.crt")
	Assert(t, (err != nil), "LocalFS TLS provider initialization with bad cert succeded while expecting it to fail")

	_, err = NewFileBasedProvider("testcerts/testServer.crt", "testcerts/testServer.key", "test.ca")
	Assert(t, (err != nil), "LocalFS TLS provider initialization with bad cert succeded while expecting it to fail")

	// this one is expected to succeed
	okProvider, err := NewFileBasedProvider("testcerts/testServer.crt", "testcerts/testServer.key", "testcerts/testCA.crt")
	AssertOk(t, err, "LocalFS TLS provider initialization failed")
	_, err = okProvider.GetServerOptions("grpc.local")
	AssertOk(t, err, "Error getting server options")
	_, err = okProvider.GetDialOptions("grpc.local")
	AssertOk(t, err, "Error getting dial options")
}
