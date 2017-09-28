// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	vchsrv "github.com/pensando/sw/venice/orch/vchub/server"
	"github.com/pensando/sw/venice/orch/vchub/sim"
	vchstore "github.com/pensando/sw/venice/orch/vchub/store"
	"github.com/pensando/sw/venice/orch/vchub/vcprobe"
	kvs "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/testutils"
)

type vchSuite struct {
	snics    []string
	vcSim    simapi.OrchSim
	vch      *vchsrv.VchServer
	vchStore *vchstore.VCHStore
	storeCh  chan defs.StoreMsg
	vcp      *vcprobe.VCProbe
}

func (vt *vchSuite) SetUp(c *C, numAgents int) {
	// create a vcsim
	vt.vcSim = sim.New()
	for ix := 0; ix < numAgents; ix++ {
		vt.snics = append(vt.snics, fmt.Sprintf("0202020202%02d", ix+1))
	}

	vcURL, err := vt.vcSim.Run("0.0.0.0:8989", vt.snics, 1)
	if err != nil {
		log.Fatalf("vcSim.Run returned %v", err)
	}

	// setup and start a vchub instance
	_, err = vchstore.Init("", kvs.KVStoreTypeMemkv)
	c.Assert(err, IsNil)

	vt.vch, err = vchsrv.NewVCHServer(vchTestURL)
	c.Assert(err, IsNil)
	vt.storeCh = make(chan defs.StoreMsg, 64)
	vt.vchStore = vchstore.NewVCHStore(context.Background())
	vt.vchStore.Run(vt.storeCh)

	u, err := soap.ParseURL(vcURL)
	c.Assert(err, IsNil)

	vt.vcp = vcprobe.NewVCProbe(u, vt.storeCh)

	AssertEventually(c, func() (bool, []interface{}) {
		if vt.vcp.Start() == nil {
			vt.vcp.Run()
			return true, nil
		}
		vt.vcp.Stop()
		return false, nil
	}, "Failed to connect to vcSim")
}

func (vt *vchSuite) TearDown() {
	vt.vcp.Stop()
	vt.vcSim.Destroy()
	vt.vch.StopServer()
	close(vt.storeCh)
	vt.vchStore.WaitForExit()
	vchstore.Close()
}
