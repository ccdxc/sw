package cmd

import (
	"fmt"
	"strings"
	"time"

	"github.com/spf13/cobra"
    "github.com/pensando/sw/api"
    "github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/iota/test/venice/iotakit"
	"github.com/pensando/sw/venice/utils/log"
)

func init() {
	rootCmd.AddCommand(veniceCmd)
	veniceCmd.AddCommand(veniceRebootCmd)
	veniceCmd.AddCommand(veniceServicesCmd)
}

var veniceCmd = &cobra.Command{
	Use:   "venice",
	Short: "actions on venice nodes",
}

var veniceRebootCmd = &cobra.Command{
	Use:   "reboot",
	Short: "reboot venice nodes, either a list of nodes or all",
	Run:   veniceRebootAction,
}

var veniceServicesCmd = &cobra.Command{
	Use:   "services",
	Short: "get the services status for the venice cluster",
	Run:   veniceServicesAction,
}

func rebootVeniceNodes(percent int) error {

	vnc, err := setupModel.VeniceNodes().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	return setupModel.Action().ReloadVeniceNodes(vnc)
}

func shutdownVeniceLeaderNode() error {
    naples := setupModel.Naples()
    leader := setupModel.VeniceNodes().Leader()
    setupModel.Action().DisconnectVeniceNodesFromCluster(leader, naples)

    // Sleep for 600 seconds to make sure we detect partitioning.
    time.Sleep(600 * time.Second)
    setupModel.Action().VerifyClusterStatus()

    //Connect Back and make sure cluster is good
    setupModel.Action().ConnectVeniceNodesToCluster(leader, naples)
    time.Sleep(60 * time.Second)

    return nil
}

func shutdownVeniceNpmNode() error {
    naples := setupModel.Naples()
    npmNode, err := setupModel.VeniceNodes().GetVeniceNodeWithService("pen-npm")
    setupModel.Action().DisconnectVeniceNodesFromCluster(npmNode, naples)

    // Sleep for 600 seconds to make sure we detect partitioning.
    time.Sleep(600 * time.Second)

    //update is add and delete of the policy
    setupModel.DefaultNetworkSecurityPolicy().Delete()
    time.Sleep(30 * time.Second)
    setupModel.DefaultNetworkSecurityPolicy().Restore()

    // verify policy was propagated correctly
    setupModel.Action().VerifyPolicyStatus(setupModel.DefaultNetworkSecurityPolicy())

    // ping all workload pairs in same subnet
    workloadPairs := setupModel.WorkloadPairs().Permit(setupModel.DefaultNetworkSecurityPolicy(), "tcp")
    setupModel.Action().TCPSession(workloadPairs, 0)

    //Connect Back and make sure cluster is good
    setupModel.Action().ConnectVeniceNodesToCluster(npmNode, naples)
    time.Sleep(60 * time.Second)

    return err
}

func shutdownVeniceApiServerNode() error {
    naples := setupModel.Naples()
    apiServerNode, err := setupModel.VeniceNodes().GetVeniceNodeWithService("pen-apiserver")
    setupModel.Action().DisconnectVeniceNodesFromCluster(apiServerNode, naples)

    // Sleep for 600 seconds to make sure we detect partitioning.
    time.Sleep(600 * time.Second)

    //update is add and delete of the policy
    setupModel.DefaultNetworkSecurityPolicy().Delete()
    setupModel.DefaultNetworkSecurityPolicy().Restore()

    // verify policy was propagated correctly
    setupModel.Action().VerifyPolicyStatus(setupModel.DefaultNetworkSecurityPolicy())

    // ping all workload pairs in same subnet
    workloadPairs := setupModel.WorkloadPairs().Permit(setupModel.DefaultNetworkSecurityPolicy(), "tcp")
    setupModel.Action().TCPSession(workloadPairs, 0)

    //Connect Back and make sure cluster is good
    setupModel.Action().ConnectVeniceNodesToCluster(apiServerNode, naples)
    time.Sleep(60 * time.Second)

    return err
}

func doVeniceOnlySnapshotRestore() error {
    err := setupModel.Action().VeniceNodeCreateSnapshotConfig(setupModel.VeniceNodes())
    ss,err := setupModel.Action().VeniceNodeTakeSnapshot(setupModel.VeniceNodes())
    name := string(ss[strings.LastIndex(ss, "/")+1:])
    setupModel.Action().VeniceNodeRestoreConfig(setupModel.VeniceNodes(), name)
    return err
}

func cleanUpVeniceNodes() {

	vnc := setupModel.VeniceNodes()

	setupModel.Action().RunCommandOnVeniceNodes(vnc, "curl -i -XDELETE http://localhost:7086/db?db=default")

	/*
		cmds := []string{}
		for _, ip := range vnc.GenVeniceIPs() {
			cmd := fmt.Sprintf("if [[ $(curl -s localhost:7086/info  | jq .tstore.NodeMap.\\\"%s\\\".NumShards) -gt 0 ]]; then  for i in  $(ls /var/lib/pensando/citadel/tstore); do curl -s localhost:7086/info  | jq .tstore.NodeMap.\\\"%s\\\".Replicas | grep -w ${i}  || rm -rf /var/lib/pensando/citadel/tstore/${i} ; done;fi",
				ip, ip)
			cmds = append(cmds, cmd)

		}
		for _, cmd := range cmds {

			setupModel.Action().RunCommandOnVeniceNodes(vnc, cmd)
		}*/
}

func partitionVeniceNode(percent int) error {

	vnc, err := setupModel.VeniceNodes().SelectByPercentage(percent)

	if err != nil {
		return err
	}

	naples := setupModel.Naples()
	err = setupModel.Action().DisconnectVeniceNodesFromCluster(vnc, naples)
	if err != nil {
		return err
	}
	time.Sleep(120 * time.Second)

	err = setupModel.Action().ConnectVeniceNodesToCluster(vnc, naples)
	if err != nil {
		return err
	}

	return nil
}

func veniceRebootAction(cmd *cobra.Command, args []string) {
	all := false
	for _, v := range args {
		if v == "all" {
			if len(args) != 1 {
				errorExit("all cannot be specified along with other nodes", nil)
			} else {
				all = true
			}
		}
	}

	if dryRun {
		log.Infof("the Following nodes would have been rebooted [%v]", args)
		return
	}

	var vn *iotakit.VeniceNodeCollection
	var err error
	if all {
		vn = setupModel.VeniceNodes()
		log.Info("All venice nodes will be rebooted")
	} else {
		selector := fmt.Sprintf("name=%s", strings.Join(args, ","))
		vn, err = setupModel.VeniceNodes().Select(selector)
		if err != nil {
			errorExit("retrieving nodes", err)
		}
		log.Infof("The following venice nodes will be rebooted %v", args)
	}
	err = setupModel.Action().ReloadVeniceNodes(vn)
	if err != nil {
		errorExit("rebooting nodes", err)
	}

	//Give it 10 minutes before we start anything.
	time.Sleep(10 * time.Minute)
}

func veniceServicesAction(cmd *cobra.Command, args []string) {
	if dryRun {
		log.Infof("skipping check for system health")
		return
	}

	log.Infof("getting cluster information")
	cl, err := setupModel.GetCluster()
	if err != nil {
		errorExit("could not retrieve cluster information", err)
	}

	status, err := setupModel.Action().GetVeniceServices()
	if err != nil {
		errorExit("retrieving cluster service status", err)
	}

	fmt.Printf("Cluster services: Leader[%v] \n%v\n", cl.Status.Leader, status)
}

func doTechSupport(percent int) error {
   var names []string
    naples := setupModel.Naples().Names()
    var nc = len(naples) * (percent / 100)
    for n := 0; n < nc; n++ {
        names = append(names, naples[n])
    }
    nodes := setupModel.VeniceNodes()
    for _,ip := range nodes.GenVeniceIPs() {
        names = append(names,ip)
    }
    techsupport := &monitoring.TechSupportRequest{
        TypeMeta: api.TypeMeta{Kind: "TechSupportRequest",},
        ObjectMeta: api.ObjectMeta{Name:"techsupport-test",},
        Spec: monitoring.TechSupportRequestSpec{
            Verbosity: 1,
            NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
                Names: names,
            },
        },
    }
    setupModel.Action().PerformTechsupport(techsupport)
    // verify techsupport is successful
    setupModel.Action().VerifyTechsupportStatus(techsupport.Name)
    return nil
}

