package vos

import (
	"testing"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/objstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCreateBuckets(t *testing.T) {
	fb := &mockBackend{}
	bmap := make(map[string]int)
	var makeErr error
	fb.makeBucketFn = func(name, loc string) error {
		if v, ok := bmap[name]; ok {
			bmap[name] = v + 1
		} else {
			bmap[name] = 1
		}
		return makeErr
	}
	err := createBuckets(fb)
	AssertOk(t, err, "create buckets failed")
	Assert(t, fb.bucketExists == len(objstore.Buckets_name), "did not find correct number of calls [%v][%v]", fb.bucketExists, len(objstore.Buckets_name))
	Assert(t, fb.makeBucket == len(objstore.Buckets_name), "did not find correct number of calls [%v][%v]", fb.makeBucket, len(objstore.Buckets_name))
	for k := range objstore.Buckets_value {
		if v1, ok := bmap["default."+k]; !ok {
			t.Errorf("did not find key [%v]", k)
		} else {
			if v1 != 1 {
				t.Errorf("duplicate calls to create buckets [%d]", v1)
			}
		}
	}

	fb.bExists = true
	fb.makeBucket = 0
	err = createBuckets(fb)
	AssertOk(t, err, "create buckets should have failed")
	Assert(t, fb.makeBucket == 0, "no calls to MakeBucket expected")

	fb.bExists = false
	makeErr = errors.New("someError")
	err = createBuckets(fb)
	Assert(t, err != nil, "create buckets should have failed")

	fb.retErr = errors.New("someError")
	err = createBuckets(fb)
	Assert(t, err != nil, "create buckets should have failed")
}
