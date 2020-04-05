package version

import (
	"testing"

	"github.com/coreos/etcd/pkg/testutil"
)

func TestValidateVersionString(t *testing.T) {
	res := ValidateVersionString("")
	testutil.AssertFalse(t, res)
	res = ValidateVersionString("1.3")
	testutil.AssertFalse(t, res)
	res = ValidateVersionString("abc")
	testutil.AssertFalse(t, res)
	res = ValidateVersionString("1.3.dirty")
	testutil.AssertTrue(t, res)
	res = ValidateVersionString("1.3-dirty.4")
	testutil.AssertFalse(t, res)
	res = ValidateVersionString("1.3.4-dirty.4")
	testutil.AssertTrue(t, res)
}

func TestGetImageType(t *testing.T) {
	imageType := GetImageType("1.3.4-E-30")
	testutil.AssertEqual(t, imageType, Enterprise)
	imageType = GetImageType("1.3.4-K-30")
	testutil.AssertEqual(t, imageType, None)
	imageType = GetImageType("1.3.4-C-30")
	testutil.AssertEqual(t, imageType, Cloud)
	imageType = GetImageType("1.3-C-30")
	testutil.AssertEqual(t, imageType, None)
	imageType = GetImageType("")
	testutil.AssertEqual(t, imageType, None)
	imageType = GetImageType("1.3")
	testutil.AssertEqual(t, imageType, None)
	imageType = GetImageType("1.3-E-33")
	testutil.AssertEqual(t, imageType, None)
	imageType = GetImageType("1.3.4-C")
	testutil.AssertEqual(t, imageType, None)
}

func TestGetMajorVersion(t *testing.T) {
	major := GetMajorVersion("1.3.1-C-10")
	testutil.AssertEqual(t, major, 1)
	major = GetMajorVersion("2.3.1")
	testutil.AssertEqual(t, major, 2)
	major = GetMajorVersion("")
	testutil.AssertEqual(t, major, 0)
	major = GetMajorVersion("1.3-C-3")
	testutil.AssertEqual(t, major, 0)
	major = GetMajorVersion("A.3.1-C-3")
	testutil.AssertEqual(t, major, 0)
}

func TestGetMinorVersion(t *testing.T) {
	minor := GetMinorVersion("1.3.1-C-10")
	testutil.AssertEqual(t, minor, 3)
	minor = GetMinorVersion("2.3.1")
	testutil.AssertEqual(t, minor, 3)
	minor = GetMinorVersion("")
	testutil.AssertEqual(t, minor, 0)
	minor = GetMinorVersion("1.3-C-3")
	testutil.AssertEqual(t, minor, 0)
	minor = GetMinorVersion("2.r.1-C-3")
	testutil.AssertEqual(t, minor, 0)
}

func TestGetRolloutType(t *testing.T) {
	rtype := GetRolloutType("1.3.1-C-10", "1.5.1-C-10")
	testutil.AssertEqual(t, rtype, Upgrade)
	rtype = GetRolloutType("2.3.1", "1.3.4")
	testutil.AssertEqual(t, rtype, Downgrade)
	rtype = GetRolloutType("iota-upgrade", "1.5.1-C-10")
	testutil.AssertEqual(t, rtype, Patch)
	rtype = GetRolloutType("1.3.1-C-10", "iota-upgrade")
	testutil.AssertEqual(t, rtype, Patch)
	rtype = GetRolloutType("1.0E", "1.5.1-C-10")
	testutil.AssertEqual(t, rtype, Patch)
	rtype = GetRolloutType("1.3.1-C-10", "1.3.1-C-10")
	testutil.AssertEqual(t, rtype, Upgrade)
}
