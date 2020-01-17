// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleNetwork(t *testing.T) {
	t.Parallel()
	network := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 32,
		},
	}

	err := HandleL2Segment(infraAPI, l2SegClient, types.Create, network, 65, []uint64{120, 121, 122})
	if err != nil {
		t.Fatal(err)
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Update, network, 65, []uint64{120, 121, 122})
	if err != nil {
		t.Fatal(err)
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, network, 65, []uint64{120, 121, 122})
	if err != nil {
		t.Fatal(err)
	}

	err = HandleL2Segment(infraAPI, l2SegClient, 42, network, 65, []uint64{120, 121, 122})
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleNetworkUntagged(t *testing.T) {
	t.Parallel()
	network := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: types.UntaggedCollVLAN,
		},
	}

	err := HandleL2Segment(infraAPI, l2SegClient, types.Create, network, 65, []uint64{120, 121, 122})
	if err != nil {
		t.Fatal(err)
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Update, network, 65, []uint64{120, 121, 122})
	if err != nil {
		t.Fatal(err)
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, network, 65, []uint64{120, 121, 122})
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkInfraFailures(t *testing.T) {
	t.Parallel()
	network := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 32,
		},
	}
	i := newBadInfraAPI()
	err := HandleL2Segment(i, l2SegClient, types.Create, network, 65, []uint64{120, 121, 122})
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleL2Segment(i, l2SegClient, types.Update, network, 65, []uint64{120, 121, 122})
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleL2Segment(i, l2SegClient, types.Delete, network, 65, []uint64{120, 121, 122})
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
