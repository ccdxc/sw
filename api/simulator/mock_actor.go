package main

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/kvstore"
)

func createIf(ctx context.Context, intf *network.NetworkInterface) {
	_, err := tinfo.apicl.NetworkV1().NetworkInterface().Create(ctx, intf)
	if err != nil {
		tinfo.l.Errorf("failed to create network interface [%v](%v)", intf.Name, apierrors.FromError(err))
	}
}

func listener(ctx context.Context) {
	// Create Smartnic objects on the APIServer
	snic := cluster.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{
			Name: "0000.CAFE.0001",
		},
		Spec: cluster.DistributedServiceCardSpec{
			ID:          "DSC-1",
			MgmtMode:    "network",
			NetworkMode: "inband",
			Controllers: []string{"localhost"},
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_PENDING.String(),
		},
	}

	_, err := tinfo.apicl.ClusterV1().DistributedServiceCard().Create(ctx, &snic)
	if err != nil {
		tinfo.l.Errorf("unable to create DSC 1 (%s)", err)
	}

	snic.Name = "0000.CAFE.0002"
	snic.Spec.ID = "DSC-2"

	_, err = tinfo.apicl.ClusterV1().DistributedServiceCard().Create(ctx, &snic)
	if err != nil {
		tinfo.l.Errorf("unable to create DSC 2 (%s)", err)
	}

	// Create a Cluster Node
	{
		node := cluster.Node{
			ObjectMeta: api.ObjectMeta{
				Name: "node1",
			},
			Status: cluster.NodeStatus{
				Phase:  cluster.NodeStatus_JOINED.String(),
				Quorum: true,
				Conditions: []cluster.NodeCondition{
					{
						Type:               cluster.NodeCondition_HEALTHY.String(),
						Status:             cluster.ConditionStatus_TRUE.String(),
						LastTransitionTime: time.Now().String(),
					},
				},
			},
		}
		_, err = tinfo.apicl.ClusterV1().Node().Create(ctx, &node)

		if err != nil {
			tinfo.l.Fatalf("Could not create the node (%s)", err)
		}
		// Create the Node interface
		intf := network.NetworkInterface{
			ObjectMeta: api.ObjectMeta{
				Name: "node1-uplink-1",
			},
			Spec: network.NetworkInterfaceSpec{
				AdminStatus: network.IFStatus_UP.String(),
				Type:        network.IFType_UPLINK_ETH.String(),
				MACAddress:  "0000.0000.0001",
				IPAllocType: network.IPAllocTypes_Static.String(),
			},
			Status: network.NetworkInterfaceStatus{
				DSC:         "",
				ClusterNode: "node1",
				Type:        network.IFType_UPLINK_ETH.String(),
				OperStatus:  network.IFStatus_UP.String(),
				PrimaryMac:  "0000.0001.0001",
			},
		}

		createIf(ctx, &intf)
	}

	deleteIfs := func(dsc string) {
		ni, err := tinfo.apicl.NetworkV1().NetworkInterface().List(ctx, &api.ListWatchOptions{})
		if err != nil {
			tinfo.l.Errorf("Could not list DSCs (%s)", err)
			return
		}
		for _, s := range ni {
			if s.Status.DSC == dsc {
				_, err = tinfo.apicl.NetworkV1().NetworkInterface().Delete(ctx, &s.ObjectMeta)
				if err != nil {
					tinfo.l.Errorf("Could not delete nework interface [%v](%s)", s.Name, err)
				}
			}
		}
	}
	watcher, err := tinfo.apicl.ClusterV1().Watch(ctx, &api.ListWatchOptions{})
	if err != nil {
		tinfo.l.Fatalf("unable start watcher")
	}
	for {
		select {
		case ev, ok := <-watcher.EventChan():
			if !ok {
				tinfo.l.Fatalf("Watcher exited, not retrying")
			}
			switch ev.Type {
			case kvstore.Created:
				// NOP
			case kvstore.Updated:
				intf := network.NetworkInterface{
					ObjectMeta: api.ObjectMeta{
						Name: "Uplink1",
					},
					Spec: network.NetworkInterfaceSpec{
						AdminStatus: network.IFStatus_UP.String(),
						Type:        network.IFType_UPLINK_ETH.String(),
						MACAddress:  "0000.0001.0001",
						IPAllocType: network.IPAllocTypes_Static.String(),
					},
					Status: network.NetworkInterfaceStatus{
						DSC:        "",
						Type:       network.IFType_UPLINK_ETH.String(),
						OperStatus: network.IFStatus_UP.String(),
						PrimaryMac: "0000.0001.0001",
					},
				}
				sn, ok := ev.Object.(*cluster.DistributedServiceCard)
				if ok {
					if sn.Spec.Admit {
						tinfo.l.Infof("DSC [%v] got admitted, creating network interfaces", sn.Name)
						snid := 1
						if strings.Contains(sn.Name, "0001") {
							snid = 1
						} else {
							snid = 2
						}
						intf.Status.DSC = sn.Name
						for i := 0; i < 2; i++ {
							// uplinks
							intf.Name = fmt.Sprintf("%v-uplink-%d", sn.Name, i+1)
							intf.Spec.MACAddress = fmt.Sprintf("0000.00%02d.00%02d", i, snid)
							intf.Status.PrimaryMac = intf.Spec.MACAddress
							intf.Status.DSC = sn.Name
							intf.Status.Name = sn.Name
							intf.Status.Type = network.IFType_UPLINK_ETH.String()
							intf.Spec.Type = network.IFType_UPLINK_ETH.String()
							createIf(ctx, &intf)
						}

						// Loopback TEP
						intf.Name = fmt.Sprintf("%v-tep-1", sn.Name)
						intf.Spec.Type = network.IFType_LOOPBACK_TEP.String()
						intf.Spec.MACAddress = ""
						intf.Status.PrimaryMac = ""
						intf.Status.DSC = sn.Name
						intf.Status.Name = sn.Name
						intf.Status.Type = network.IFType_LOOPBACK_TEP.String()
						createIf(ctx, &intf)

						for i := 0; i < 6; i++ {
							intf.Name = fmt.Sprintf("%v-pf-%d", sn.Name, i+1)
							intf.Spec.Type = network.IFType_HOST_PF.String()
							intf.Status.PrimaryMac = fmt.Sprintf("00%02d.BEEF.00%02d", snid, i)
							intf.Status.Type = network.IFType_HOST_PF.String()
							intf.Status.DSC = sn.Name
							intf.Status.Name = sn.Name
							intf.Status.Type = network.IFType_HOST_PF.String()
							createIf(ctx, &intf)
						}
					} else {
						tinfo.l.Infof("got update with admit as false, deleting interfaces")
						deleteIfs(sn.Name)
					}
				}
			case kvstore.Deleted:

				sn, ok := ev.Object.(*cluster.DistributedServiceCard)
				if ok {
					tinfo.l.Infof("Got delete notification for [%v]", sn.Name)
					deleteIfs(sn.Name)
				}

			}
		case <-ctx.Done():
			tinfo.l.Infof("got context cancel, exiting")
		}
	}
}
