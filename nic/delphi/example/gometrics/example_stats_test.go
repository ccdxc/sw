// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package gostats

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/nic/delphi/proto/goproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestMetricsSpecific(t *testing.T) {
	iter, err := goproto.NewExampleMetricsIterator()
	AssertOk(t, err, "Error creating metrics iterator")
	if iter == nil {
		// skip the test on osx
		return
	}

	// create an entry
	tmtr, err := iter.Create(3000)
	AssertOk(t, err, "Error creating test metrics entry")
	fmt.Printf("Created metrics: %+v\n", tmtr)

	// set some values
	tmtr.SetRxPkts(200)
	tmtr.SetTxPkts(300)
	tmtr.SetRxPktRate(400.0)
	tmtr.SetTxPktRate(500.0)

	tmtr, err = iter.Find(3000)
	AssertOk(t, err, "Error finding test metrics entry")
	fmt.Printf("Found metrics: %+v\n", tmtr)
	Assert(t, (tmtr.RxPkts == 200), "Invalid counter value")
	Assert(t, (tmtr.RxPktRate == 400.0), "Invalid gauge value")
	Assert(t, (tmtr.GetKey() == 3000), "Invalid key value")

	iter, err = goproto.NewExampleMetricsIterator()
	count := 0
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("new TestMetrics: %+v\n", mtr)
		Assert(t, (mtr.GetKey() == 3000), "Invalid key value")
		count++
	}
	Assert(t, (count == 1), "Iterator found invalid objects", count)

	// delete the metrics
	iter.Delete(3000)
}
