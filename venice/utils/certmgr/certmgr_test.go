// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"testing"

	"github.com/pensando/sw/venice/utils/keymgr"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCertMgrInit(t *testing.T) {
	// NEGATIVE TEST-CASES

	// empty dir
	_, err := NewCertificateMgr(nil)
	Assert(t, err != nil, "NewCertificateAuthority succeeded with nil KeyMgr instance")

	// POSITIVE TEST-CASE
	be, err := keymgr.NewDefaultBackend()
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	cm, err := NewCertificateMgr(km)
	AssertOk(t, err, "Error instantiating CertificateMgr")
	Assert(t, cm.IsReady(), "CertificateMgr not ready")
	Assert(t, nil != cm.Ca(), "CertificateMgr returned a nil CA")
	Assert(t, cm.Ca().IsReady(), "CertificateMgr CA not ready")
}
