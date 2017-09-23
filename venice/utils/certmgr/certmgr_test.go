// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"io/ioutil"
	"os"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCertMgrInit(t *testing.T) {
	// NEGATIVE TEST-CASES

	// empty dir
	_, err := NewCertificateMgr("")
	Assert(t, err != nil, "NewCertificateAuthority succeeded with empty dir")

	// path exists but is a file
	tmpfile, err := ioutil.TempFile("", "certmgr")
	AssertOk(t, err, "Error creating temporary file")
	defer os.RemoveAll(tmpfile.Name())
	_, err = NewCertificateMgr(tmpfile.Name())
	Assert(t, err != nil, "NewCertificateMgr succeeded with invalid dir")

	// POSITIVE TES-CASE
	dir, err := ioutil.TempDir("", "certmgrtest")
	AssertOk(t, err, "Error creating temporary directory")
	cm, err := NewCertificateMgr(dir)
	AssertOk(t, err, "Error instantiating CertificateMgr")
	Assert(t, cm.IsReady(), "CertificateMgr not ready")
	Assert(t, nil != cm.Ca(), "CertificateMgr returned a nil CA")
	Assert(t, cm.Ca().IsReady(), "CertificateMgr CA not ready")
}
