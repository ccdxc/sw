// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/agent/netagent/datapath"
	"github.com/pensando/sw/agent/plugins/k8s/cni"
)

/* Rough Architecture for Pensando Agent
 * -------------------------------------------
 *
 *                Kubelet     Docker     Kubelet     Docker
 *                   |          |            |         |
 *                   V          V            V         V
 *               +---------+------------+---------+------------+
 *               | K8s Net | Docker Net | K8s Vol | Docker Vol |
 *               | Plugin  | Plugin     | Plugin  | Plugin     |
 *               +---------+------------+---------+------------+
 *                        |                      |
 *                   NetAgentAPI             VolAgentAPI
 *                        |                      |
 *                        V                      V
 *               +----------------------+----------------------+
 *              |                      |                      |
 *  Network <-->+      NetAgent        |      VolAgent        +<--> Volume
 *  Controller  |                      |                      |     Controller
 *                 +---------+------------+---------+------------+
 *                   |           |     \              |
 *               NetData    SvcData     SecData    VolData
 *               pathAPI    pathAPI     pathAPI    pathAPI
 *                   |           |          |         |
 *                   V           V          V         V
 *               +----------+-----------+----------+-----------+
 *               | Network  | Service   | Security | Volume    |
 *               | Datapath | Datapath  | Datapath | Datapath  |
 *               +---------+------------+---------+------------+
 *
 */
// Main function
func main() {
	// create a dummy channel to wait forver
	waitCh := make(chan bool)

	// create a network datapath
	dp, err := datapath.NewFakeDatapath()
	if err != nil {
		log.Fatalf("Error creating fake datapath. Err: %v", err)
	}

	// create new network agent
	nagent, err := netagent.NewNetAgent(dp)
	if err != nil {
		log.Fatalf("Error creating network agent. Err: %v", err)
	}

	// create a CNI server
	cniServer, err := cni.NewCniServer(cni.CniServerListenURL, nagent)
	if err != nil {
		log.Fatalf("Error creating CNI server. Err: %v", err)
	}

	log.Printf("CNI server {%+v} is running", cniServer)
	// wait forever
	<-waitCh
}
