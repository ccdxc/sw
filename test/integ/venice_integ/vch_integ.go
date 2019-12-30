// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	check "gopkg.in/check.v1"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
)

type vchSuite struct {
	storeCh chan defs.Probe2StoreMsg
	vcp     *vcprobe.VCProbe
}

func (vt *vchSuite) SetUp(c *check.C, numAgents int) {
	// TODO: Implement
}

func (vt *vchSuite) TearDown() {
}
