package pkg

import (
	"fmt"
	"strings"
	"testing"

	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestTraffic(t *testing.T) {

	var trafficCnt = 0
	testhpingRun := func(cmd []string) error {
		fmt.Println(strings.Join(cmd, " "))
		trafficCnt++
		return nil
	}

	hpingRun = testhpingRun
	err := RunTraffic("./test/uplink_map.cfg", "./test/e2e.cfg", TrafficUplinkToUplink)
	fmt.Println(trafficCnt)
	TestUtils.Assert(t, err == nil && trafficCnt == 4, "Test failed")

}
