// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package tokenauth

import (
	"reflect"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/tokenauth/readutils"

	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/keymgr"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestMakeNodeToken(t *testing.T) {
	be, err := keymgr.NewDefaultBackend("certmgr")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()

	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")
	ca, err := certmgr.NewCertificateAuthority(km, true)
	AssertOk(t, err, "Error instantiating certificate authority")

	type TestCase struct {
		clusterName string
		audience    []string
		notBefore   time.Time
		notAfter    time.Time
		success     bool
	}

	notBefore := time.Now()
	notAfter := notBefore.Add(time.Hour)
	timeZero := time.Time{}

	tcs := []TestCase{
		{"", nil, timeZero, timeZero, false},
		{"", nil, notBefore, notAfter, true},
		{"", nil, notBefore, notAfter, true},
		{"", []string{"aud1"}, notBefore, notAfter, true},
		{"", []string{"aud1"}, notAfter, notBefore, false}, // notBefore and notAfter swapped
		{"", []string{"aud1", "aud2=Y"}, notBefore, notAfter, true},
		{"", []string{"*"}, notBefore, notAfter, true},
		{"tc", nil, notBefore, notAfter, true},
		{"tc", nil, notBefore, notAfter, true},
		{"tc", []string{"aud1"}, notBefore, notAfter, true},
		{"tc", []string{"*"}, notBefore, notAfter, true},
	}

	for i, tc := range tcs {
		token, err := MakeNodeToken(ca, tc.clusterName, tc.audience, tc.notBefore, tc.notAfter)
		Assert(t, (err == nil && tc.success == true) || (err != nil && tc.success == false),
			"Testcase %d, unexpected error: %v", i, err)

		if err == nil {
			outAudience, err := readutils.GetNodeTokenAttributes(token)
			AssertOk(t, err, "Testcase %d, error parsing token: %v", i, err)
			Assert(t, (len(tc.audience) == 0 && len(outAudience) == 0) || reflect.DeepEqual(outAudience, tc.audience),
				"Testcase %d. audience mismatch. Have: %#v, want: %#v", i, outAudience, tc.audience)
		}
	}
}
