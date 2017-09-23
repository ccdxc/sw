// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"testing"

	"github.com/pensando/sw/venice/utils/certmgr"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestRPCServerInit(t *testing.T) {
	// NEGATIVE TEST-CASES
	var cm *certmgr.CertificateMgr
	// no server name
	_, err := NewRPCServer("", "bar", cm)
	Assert(t, (err != nil), "RPCServer init succeeded while expected to fail")
	// no listenURL name
	_, err = NewRPCServer("foo", "", cm)
	Assert(t, (err != nil), "RPCServer init succeeded while expected to fail")
	// no certificate mgr
	_, err = NewRPCServer("foo", "bar", nil)
	Assert(t, (err != nil), "RPCServer init succeeded while expected to fail")
}
