// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	check "gopkg.in/check.v1"

	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	vchstore "github.com/pensando/sw/venice/orch/vchub/store"
	"github.com/pensando/sw/venice/orch/vchub/vcprobe"
)

type vchSuite struct {
	snics    []string
	vcSim    simapi.OrchSim
	vchStore *vchstore.VCHStore
	storeCh  chan defs.StoreMsg
	vcp      *vcprobe.VCProbe
}

func (vt *vchSuite) SetUp(c *check.C, numAgents int) {
	// TODO: Implement
}

func (vt *vchSuite) TearDown() {
}
