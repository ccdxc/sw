package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"

	"github.com/pkg/errors"
	cobra "github.com/spf13/cobra"
	"google.golang.org/grpc"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Globals "github.com/pensando/sw/iota/svcs/common"
)

var (
	nodeFile, iotaSvcIP                          string
	iotaSvcPort                                  int
	perosanlity, nodeName, workloadName, cfgFile string
	add, delete                                  bool
)

//newIotaAgentClient returns instance of new app agent client.
func newIotaAgentClient(ip string, port int) (iota.IotaAgentApiClient, error) {
	var conn *grpc.ClientConn
	conn, err := grpc.Dial(ip+":"+strconv.Itoa(port), grpc.WithInsecure())
	if err != nil {
		return nil, errors.Wrap(err, "Agent connection failed!")
	}
	return iota.NewIotaAgentApiClient(conn), nil
}

func getNode(nodeFile string, nodeName string) (*iota.Node, error) {

	var iotaNode *iota.Node
	file, e := ioutil.ReadFile(nodeFile)
	if e != nil {
		return nil, errors.Wrap(e, "Error opening node file")
	}

	var nodesData map[string]interface{}
	var err error

	err = json.Unmarshal(file, &nodesData)
	if err != nil {
		return nil, errors.Wrap(err, "Error in unmarshalling topo file")
	}

	if _, ok := nodesData[nodeName]; !ok {
		return nil, errors.New("Node name not found")
	}

	nodeData := nodesData[nodeName].(map[string]interface{})

	if nodeData["kind"] == "naples" {
		iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_NAPLES,
			Name: nodeName, Image: "naples-release-v1.tgz"}
		dataIntfs := []string{}
		dataIPs := []string{}
		naplesIPs := []string{}
		for _, intf := range nodeData["data-intfs"].([]interface{}) {
			dataIntfs = append(dataIntfs, intf.(string))
		}
		if _, ok := nodeData["data-ips"]; ok {
			for _, intf := range nodeData["data-ips"].([]interface{}) {
				dataIPs = append(dataIPs, intf.(string))
			}

			for _, intf := range nodeData["naples-ips"].([]interface{}) {
				naplesIPs = append(naplesIPs, intf.(string))
			}
		}

		iotaNode.NodeInfo = &iota.Node_NaplesConfigs{&iota.NaplesConfigs{Configs: []*iota.NaplesConfig{&iota.NaplesConfig{
			ControlIntf: nodeData["control-intf"].(string),
			DataIntfs:   dataIntfs,
			ControlIp:   nodeData["control-ip"].(string),
		}}}}

	} else if nodeData["kind"] == "venice" {
		veniceNodes := ([]*iota.VenicePeer{})
		for _, data := range nodeData["venice-nodes"].([]interface{}) {
			nodeInfo := data.(map[string]interface{})
			node := &iota.VenicePeer{HostName: nodeInfo["hostname"].(string),
				IpAddress: nodeInfo["ip"].(string)}
			veniceNodes = append(veniceNodes, node)
		}
		iotaNode = &iota.Node{Type: iota.PersonalityType_PERSONALITY_VENICE, Name: nodeName, Image: "venice.tgz"}
		iotaNode.NodeInfo = &iota.Node_VeniceConfig{VeniceConfig: &iota.VeniceConfig{
			ControlIntf: nodeData["control-intf"].(string),
			ControlIp:   nodeData["control-ip"].(string)}}
	} else {
		return nil, errors.New("Node type not supported")
	}

	return iotaNode, nil
}

func getWorkload(nodeFile string, nodeName string, workloadName string) (*iota.Workload, error) {

	file, e := ioutil.ReadFile(nodeFile)
	if e != nil {
		return nil, errors.Wrap(e, "Error opening node file")
	}

	var nodesData map[string]interface{}
	var err error

	err = json.Unmarshal(file, &nodesData)
	if err != nil {
		return nil, errors.Wrap(err, "Error in unmarshalling topo file")
	}

	if _, ok := nodesData[nodeName]; !ok {
		return nil, errors.New("Node name not found")
	}

	nodeData := nodesData[nodeName].(map[string]interface{})

	if nodeData["kind"] == "naples" {

		workloads := nodeData["workloads"].(map[string]interface{})
		if _, ok := workloads[workloadName]; !ok {
			return nil, errors.New("Workload not found")
		}

		workloadData := workloads[workloadName].(map[string]interface{})
		return &iota.Workload{NodeName: nodeName, WorkloadName: workloadName,
			EncapVlan: uint32(workloadData["encap-vlan"].(float64)), Interface: workloadData["interface"].(string),
			IpPrefix: workloadData["ip-address"].(string), MacAddress: workloadData["mac-address"].(string),
			UplinkVlan: uint32(workloadData["uplink-vlan"].(float64)),
			PinnedPort: uint32(workloadData["pinned-port"].(float64))}, nil
	}

	return nil, errors.New("Node type workload add not supported")
}

var personalityCmd = &cobra.Command{
	Use:   "personality",
	Short: "Add or set personality",
	RunE: func(cmd *cobra.Command, args []string) error {

		if !add && !delete {
			return errors.New("No operation specified")
		}

		if nodeName == "" {
			return errors.New("No nodename  specified")
		}

		agentClient, err := newIotaAgentClient(iotaSvcIP, iotaSvcPort)
		if err != nil {
			return errors.Wrap(err, "Iota Svc not up")
		}

		iotaNode, err := getNode("node.json", nodeName)

		if err != nil {
			return errors.Wrap(err, "Node not found")
		}

		if add {
			resp, _ := agentClient.AddNode(context.Background(), iotaNode)
			if resp.NodeStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
				return errors.New("Add node failed")
			}
		} else {
			resp, _ := agentClient.DeleteNode(context.Background(), iotaNode)
			if resp.NodeStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
				return errors.New("Add node failed")
			}
		}
		if err != nil {
			return errors.Wrap(err, "Add Node call failed")
		}

		fmt.Println("Successfully did clean up and init of test bed")
		return nil
	},
}

var healthCmd = &cobra.Command{
	Use:   "health",
	Short: "check health of node",
	RunE: func(cmd *cobra.Command, args []string) error {

		if nodeName == "" {
			return errors.New("No nodename  specified")
		}

		agentClient, err := newIotaAgentClient(iotaSvcIP, iotaSvcPort)
		if err != nil {
			return errors.Wrap(err, "Iota Svc not up")
		}

		_, err = getNode("node.json", nodeName)
		if err != nil {
			return err
		}

		iotaNodeHealth := &iota.NodeHealth{NodeName: nodeName}

		iotaNodeHealth, err = agentClient.CheckHealth(context.Background(), iotaNodeHealth)
		if err != nil {
			return errors.Wrap(err, "Node health call failed!")
		}

		if iotaNodeHealth.GetHealthCode() != iota.NodeHealth_HEALTH_OK {
			fmt.Println("Node health not ok!")
		} else {
			fmt.Println("Node health ok!")
		}

		return nil
	},
}

var workloadCmd = &cobra.Command{
	Use:   "workload",
	Short: "Add or delete workload",
	RunE: func(cmd *cobra.Command, args []string) error {

		if !add && !delete {
			return errors.New("No operation specified")
		}

		if nodeName == "" {
			return errors.New("No nodename  specified")
		}

		agentClient, err := newIotaAgentClient(iotaSvcIP, iotaSvcPort)
		if err != nil {
			return errors.Wrap(err, "Iota Svc not up")
		}

		workload, err := getWorkload("node.json", nodeName, workloadName)

		if err != nil {
			return errors.Wrap(err, "workload not found")
		}

		if add {
			workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{workload}}
			resp, _ := agentClient.AddWorkloads(context.Background(), workloadMsg)
			if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
				return errors.New("Add workload failed")
			}
		} else {
			workloadMsg := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{workload}}
			resp, _ := agentClient.DeleteWorkloads(context.Background(), workloadMsg)
			if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
				return errors.New("Delete workload failed")
			}
		}

		fmt.Println("Successfully Completed operation")
		return nil
	},
}

var configCmd = &cobra.Command{
	Use:   "config",
	Short: "Push configuration",
	RunE: func(cmd *cobra.Command, args []string) error {

		cfg := GetAgentConfig(cfgFile)

		if cfg == nil {
			return errors.New("Could not read configuraiton")
		}

		ConfigureNaples(cfg)

		fmt.Println("Configured naples")
		return nil
	},
}

// RootCmd represents the base command when called without any subcommands
var RootCmd = &cobra.Command{
	Use:   "iota-agent-client",
	Short: "Test agent client to do some tests",
	Run: func(cmd *cobra.Command, args []string) {
		cmd.Usage()
	},
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	RootCmd.AddCommand(personalityCmd)
	RootCmd.AddCommand(healthCmd)
	RootCmd.AddCommand(workloadCmd)
	RootCmd.AddCommand(configCmd)
	if err := RootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func main() {
	Execute()
}

func init() {
	personalityCmd.Flags().StringVarP(&perosanlity, "personality", "t", "", "Personality type")
	personalityCmd.Flags().StringVarP(&nodeName, "name", "", "", "Node name in test json")
	personalityCmd.Flags().BoolVarP(&add, "add", "a", false, "Add node personality")
	personalityCmd.Flags().BoolVarP(&delete, "delete", "d", false, "Delete node personality")
	personalityCmd.Flags().StringVarP(&iotaSvcIP, "iota-ip", "i", "localhost", "Iota Svc IP")
	personalityCmd.Flags().IntVarP(&iotaSvcPort, "iota-port", "p", Globals.IotaAgentPort, "Iota agent port")
	healthCmd.Flags().StringVarP(&nodeName, "name", "", "", "Node name in test json")
	healthCmd.Flags().StringVarP(&iotaSvcIP, "iota-ip", "i", "localhost", "Iota Svc IP")
	healthCmd.Flags().IntVarP(&iotaSvcPort, "iota-port", "p", Globals.IotaAgentPort, "Iota agent port")

	workloadCmd.Flags().StringVarP(&nodeName, "node", "", "", "Node name in test json")
	workloadCmd.Flags().StringVarP(&workloadName, "name", "", "", "Workload name under the node")
	workloadCmd.Flags().BoolVarP(&add, "add", "a", false, "Add workload")
	workloadCmd.Flags().BoolVarP(&delete, "delete", "d", false, "Delete workload")
	workloadCmd.Flags().StringVarP(&iotaSvcIP, "iota-ip", "i", "localhost", "Iota Svc IP")
	workloadCmd.Flags().IntVarP(&iotaSvcPort, "iota-port", "p", Globals.IotaAgentPort, "Iota agent port")

	configCmd.Flags().StringVarP(&cfgFile, "cfgfile", "", "", "Cfg file to push")
}
