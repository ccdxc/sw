// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// GetVeniceURL returns venice URL for the testbed
func (tb *TestBed) GetVeniceURL() []string {
	var veniceURL []string

	if tb.mockMode {
		return []string{mockVeniceURL}
	}

	// walk all venice nodes
	for _, node := range tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			veniceURL = append(veniceURL, fmt.Sprintf("%s:%s", node.NodeMgmtIP, globals.APIGwRESTPort))
		}
	}

	return veniceURL
}

// getVeniceHostNames returns a list of venice host names
func (tb *TestBed) getVeniceHostNames() []string {
	var hostNames []string

	// walk all venice nodes
	for _, node := range tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			hostNames = append(hostNames, node.NodeName)
		}
	}

	return hostNames
}

// getVeniceIPAddrs returns list of Venic enode IP addresses
func (tb *TestBed) getVeniceIPAddrs() []string {
	var veniceIPs []string

	// walk all venice nodes
	for _, node := range tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			veniceIPs = append(veniceIPs, node.NodeMgmtIP)
		}
	}

	return veniceIPs
}

// CheckIotaClusterHealth checks iota cluster health
func (tb *TestBed) CheckIotaClusterHealth() error {
	// check cluster health
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	tb.addNodeResp.ClusterDone = tb.makeClustrResp != nil
	healthResp, err := topoClient.CheckClusterHealth(context.Background(), tb.addNodeResp)
	if err != nil {
		log.Errorf("Failed to check health of the cluster. Err: %v", err)
		return fmt.Errorf("Cluster health check failed. %v", err)
	} else if healthResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to check health of the cluster {%+v}. Err: %v", healthResp.ApiResponse, err)
		return fmt.Errorf("Cluster health check failed %v", healthResp.ApiResponse.ApiStatus)
	}

	for _, h := range healthResp.Health {
		if h.HealthCode != iota.NodeHealth_HEALTH_OK {
			log.Errorf("Testbed unhealthy. HealthCode: %v | Node: %v", h.HealthCode, h.NodeName)
			return fmt.Errorf("Cluster health check failed")
		}
	}

	log.Debugf("Got cluster health resp: %+v", healthResp)

	return nil
}

// MakeVeniceCluster inits the venice cluster
func (sm *SysModel) MakeVeniceCluster(ctx context.Context) error {
	// get CMD URL URL
	var veniceCmdURL []string
	for _, node := range sm.tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			veniceCmdURL = append(veniceCmdURL, fmt.Sprintf("%s:9001", node.NodeMgmtIP))
		}
	}

	// cluster message to init cluster
	clusterCfg := cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: "Cluster"},
		ObjectMeta: api.ObjectMeta{
			Name: "iota-cluster",
		},
		Spec: cluster.ClusterSpec{
			AutoAdmitDSCs: true,
			QuorumNodes:   sm.tb.getVeniceIPAddrs(),
		},
	}

	// make cluster message to be sent to API server
	clusterStr, _ := json.Marshal(clusterCfg)
	makeCluster := iota.MakeClusterMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Endpoint:    veniceCmdURL[0] + "/api/v1/cluster",
		Config:      string(clusterStr),
	}

	log.Infof("Making Venice cluster..")

	// ask iota server to make cluster
	log.Debugf("Making cluster with params: %+v", makeCluster)
	cfgClient := iota.NewConfigMgmtApiClient(sm.tb.iotaClient.Client)
	resp, err := cfgClient.MakeCluster(ctx, &makeCluster)
	if err != nil {
		log.Errorf("Error initing venice cluster. Err: %v", err)
		return err
	}
	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Error making venice cluster: ApiResp: %+v. Err %v", resp.ApiResponse, err)
		return fmt.Errorf("Error making venice cluster")
	}
	sm.tb.makeClustrResp = resp

	// done
	return nil
}

// SetupVeniceNodes sets up some test tools on venice nodes
func (sm *SysModel) SetupVeniceNodes() error {

	log.Infof("Setting up venice nodes..")

	// walk all venice nodes
	trig := sm.tb.NewTrigger()
	for _, node := range sm.tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			entity := node.NodeName + "_venice"
			trig.AddCommand(fmt.Sprintf("mkdir -p /pensando/iota/k8s/"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("sudo cp -r /var/lib/pensando/pki/kubernetes/apiserver-client /pensando/iota/k8s/"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("sudo chmod -R 777 /pensando/iota/k8s/"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("mkdir -p /pensando/iota/bin; docker run -v /pensando/iota/bin:/import registry.test.pensando.io:5000/pens-debug:v0.1"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf(`echo '/pensando/iota/bin/kubectl config set-cluster e2e --server=https://%s:6443 --certificate-authority=/pensando/iota/k8s/apiserver-client/ca-bundle.pem;
				/pensando/iota/bin/kubectl config set-context e2e --cluster=e2e --user=admin;
				/pensando/iota/bin/kubectl config use-context e2e;
				/pensando/iota/bin/kubectl config set-credentials admin --client-certificate=/pensando/iota/k8s/apiserver-client/cert.pem --client-key=/pensando/iota/k8s/apiserver-client/key.pem;
				' > /pensando/iota/setup_kubectl.sh
				`, node.NodeMgmtIP), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("chmod +x /pensando/iota/setup_kubectl.sh"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("/pensando/iota/setup_kubectl.sh"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf(`echo 'docker run --rm --name kibana --network host \
				-v /var/lib/pensando/pki/shared/elastic-client-auth:/usr/share/kibana/config/auth \
				-e ELASTICSEARCH_URL=https://%s:9200 \
				-e ELASTICSEARCH_SSL_CERTIFICATEAUTHORITIES="config/auth/ca-bundle.pem" \
				-e ELASTICSEARCH_SSL_CERTIFICATE="config/auth/cert.pem" \
				-e ELASTICSEARCH_SSL_KEY="config/auth/key.pem" \
				-e xpack.security.enabled=false \
				-e xpack.logstash.enabled=false \
				-e xpack.graph.enable=false \
				-e xpack.watcher.enabled=false \
				-e xpack.ml.enabled=false \
				-e xpack.monitoring.enabled=false \
				-d docker.elastic.co/kibana/kibana:6.3.0
				' > /pensando/iota/start_kibana.sh
				`, node.NodeName), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("chmod +x /pensando/iota/start_kibana.sh"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("rm /etc/localtime"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("ln -s /usr/share/zoneinfo/US/Pacific /etc/localtime"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("docker run -d --name=grafana --net=host -e \"GF_SECURITY_ADMIN_PASSWORD=password\" registry.test.pensando.io:5000/pensando/grafana:0.1"), entity, node.NodeName)

		}
	}

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to setup venice node. Err: %v", err)
		return fmt.Errorf("Error triggering commands on venice nodes: %v", err)
	}

	for _, cmdResp := range triggerResp {
		// 'echo' command sometimes has exit code 1. ignore it
		if cmdResp.ExitCode != 0 && !strings.HasPrefix(cmdResp.Command, "echo") {
			return fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

//CheckCitadelServiceStatus check citadel status
func (sm *SysModel) CheckCitadelServiceStatus() error {

	// walk all venice nodes
	trig := sm.tb.NewTrigger()
	for _, node := range sm.veniceNodes {
		entity := node.iotaNode.Name + "_venice"
		trig.AddCommand(fmt.Sprintf(`curl  http://localhost:7086/query --data-urlencode "db=default" --data-urlencode "q=SELECT * FROM Node" `),
			entity, node.iotaNode.Name)
	}

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run citadel status status. Err: %v", err)
		return err
	}

	for _, cmdResp := range triggerResp {
		if cmdResp.ExitCode != 0 {
			return fmt.Errorf("Venice trigger for citadel check failed %v failed. code %v, Out: %v, StdErr: %v",
				cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// CheckVeniceServiceStatus checks if all services are running on venice nodes
func (sm *SysModel) CheckVeniceServiceStatus(leaderNode string) (string, error) {
	ret := ""
	trig := sm.tb.NewTrigger()
	for _, node := range sm.veniceNodes {
		entity := node.iotaNode.Name + "_venice"
		trig.AddCommand(fmt.Sprintf("docker ps -q -f Name=pen-cmd"), entity, node.iotaNode.Name)
		trig.AddCommand(fmt.Sprintf("docker ps -q -f Name=pen-etcd"), entity, node.iotaNode.Name)
	}

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to check cmd/etcd service status. Err: %v", err)
		return ret, err
	}

	for _, cmdResp := range triggerResp {
		if cmdResp.ExitCode != 0 {
			return ret, fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
		if cmdResp.Stdout == "" {
			return ret, fmt.Errorf("Venice required service not running: %v", cmdResp.Command)
		}
	}

	// check all pods on leader node
	for _, node := range sm.veniceNodes {
		if node.iotaNode.IpAddress == leaderNode {
			trig = sm.tb.NewTrigger()
			entity := node.iotaNode.Name + "_venice"
			trig.AddCommand(fmt.Sprintf("/pensando/iota/bin/kubectl get pods -owide --no-headers"), entity, node.iotaNode.Name)

			// trigger commands
			triggerResp, err = trig.Run()
			if err != nil {
				log.Errorf("Failed to get k8s service status Err: %v", err)
				return ret, err
			}

			for _, cmdResp := range triggerResp {
				if cmdResp.ExitCode != 0 {
					return ret, fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
				}
				if cmdResp.Stdout == "" {
					return ret, fmt.Errorf("Could not get any information from k8s: %v", cmdResp.Command)
				}
				log.Debugf("Got kubectl resp\n%v", cmdResp.Stdout)
				ret = cmdResp.Stdout
				out := strings.Split(cmdResp.Stdout, "\n")
			checkLoop:
				for _, line := range out {
					if line != "" && !strings.Contains(line, "Running") {
						for _, downNode := range sm.veniceNodesDisconnected {
							if strings.Contains(line, downNode.iotaNode.IpAddress) {
								//Ignore the nodes which are disconnected.
								break checkLoop
							}
						}
						fmt.Printf("Some kuberneted services were not running: %v", cmdResp.Stdout)
						return ret, fmt.Errorf("Some pods not running: %v", line)
					}
				}
			}

			cNodes := []string{}
			for _, vn := range sm.veniceNodes {
				cNodes = append(cNodes, fmt.Sprintf("%q", vn.iotaNode.IpAddress))
			}

			hostSelector := "select([.status.hostIP]|inside([" + strings.Join(cNodes, ",") + "]))"
			trig = sm.tb.NewTrigger()

			trig.AddCommand(`/pensando/iota/bin/kubectl get pods -a --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | `+hostSelector+
				`| select(.status.phase != "Running" or ([ .status.conditions[] | select(.type == "Ready" and .status == "False") ] | length ) == 1 ) | .metadata.namespace + "/" + .metadata.name' `,
				entity, node.iotaNode.Name)

			// trigger commands
			triggerResp, err = trig.Run()
			if err != nil {
				log.Errorf("Failed to get k8s service status Err: %v", err)
				return ret, err
			}

			for _, cmdResp := range triggerResp {
				if cmdResp.ExitCode != 0 {
					return ret, fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
				}
				if cmdResp.Stdout != "" {
					fmt.Printf("Some pods not ready: %v, ignoring....", cmdResp.Stdout)
				}
			}
		}

	}
	return ret, nil
}

// CheckNaplesHealth checks if naples is healthy
func (sm *SysModel) CheckNaplesHealth(node *Naples) error {
	nodeIP := node.testNode.instParams.NodeMgmtIP
	if node.testNode.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
		nodeIP = node.testNode.instParams.NicMgmtIP
	}

	// get naples status from NMD
	// Note: struct redefined here to avoid dependency on NMD packages
	var naplesStatus struct {
		api.TypeMeta   `protobuf:"bytes,1,opt,name=T,embedded=T" json:",inline"`
		api.ObjectMeta `protobuf:"bytes,2,opt,name=O,embedded=O" json:"meta,omitempty"`
		Spec           struct {
			PrimaryMAC  string   `protobuf:"bytes,1,opt,name=PrimaryMAC,proto3" json:"primary-mac,omitempty"`
			Hostname    string   `protobuf:"bytes,2,opt,name=Hostname,proto3" json:"hostname,omitempty"`
			Mode        string   `protobuf:"bytes,4,opt,name=Mode,proto3" json:"mode"`
			NetworkMode string   `protobuf:"bytes,5,opt,name=NetworkMode,proto3" json:"network-mode"`
			MgmtVlan    uint32   `protobuf:"varint,6,opt,name=MgmtVlan,proto3" json:"vlan,omitempty"`
			Controllers []string `protobuf:"bytes,7,rep,name=Controllers" json:"controllers,omitempty"`
			Profile     string   `protobuf:"varint,8,opt,name=Profile,proto3,enum=nmd.NaplesSpec_FeatureProfile" json:"feature-profile,omitempty"`
		}
		Status struct {
			Phase           string   `protobuf:"varint,1,opt,name=Phase,proto3,enum=cluster.SmartNICStatus_Phase" json:"phase,omitempty"`
			Controllers     []string `protobuf:"bytes,3,rep,name=Controllers" json:"controllers,omitempty"`
			TransitionPhase string   `protobuf:"bytes,4,opt,name=TransitionPhase,proto3" json:"transition-phase,omitempty"`
			Mode            string   `protobuf:"bytes,5,opt,name=Mode,proto3" json:"mode"`
			NetworkMode     string   `protobuf:"bytes,6,opt,name=NetworkMode,proto3" json:"network-mode"`
		}
	}

	// NAPLES is supposed to be part of a Cluster, so we need auth token to talk to Agent
	veniceCtx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		nerr := fmt.Errorf("Could not get Venice logged in context: %v", err)
		log.Errorf("%v", nerr)
		return nerr
	}
	ctx, cancel := context.WithTimeout(veniceCtx, 5*time.Second)
	defer cancel()
	agentClient, err := utils.GetNodeAuthTokenHTTPClient(ctx, sm.GetVeniceURL()[0], []string{"*"})
	if err != nil {
		nerr := fmt.Errorf("Could not get naples authenticated client from Venice: %v", err)
		log.Errorf("%v", nerr)
		return nerr
	}
	status, err := agentClient.Req("GET", "https://"+nodeIP+":8888/api/v1/naples/", nil, &naplesStatus)
	if err != nil || status != http.StatusOK {
		nerr := fmt.Errorf("Could not get naples status from NMD. Status: %v, err: %v", status, err)
		log.Errorf("%v", nerr)
		return nerr
	}

	// check naples status
	if naplesStatus.Spec.Mode != "NETWORK" || naplesStatus.Spec.NetworkMode != "OOB" {
		nerr := fmt.Errorf("Invalid NMD mode configuration: %+v", naplesStatus.Spec)
		log.Errorf("%v", nerr)
		return nerr
	}
	if node.testNode.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
		if !strings.Contains(naplesStatus.Status.TransitionPhase, "REGISTRATION_DONE") {
			nerr := fmt.Errorf("Invalid NMD phase: %v", naplesStatus.Status.TransitionPhase)
			log.Errorf("%v", nerr)
			return nerr
		}
	} else {
		if !strings.Contains(naplesStatus.Status.TransitionPhase, "REGISTRATION_DONE") && !strings.Contains(naplesStatus.Status.TransitionPhase, "REBOOT_PENDING") {
			nerr := fmt.Errorf("Invalid NMD phase: %v", naplesStatus.Status.TransitionPhase)
			log.Errorf("%v", nerr)
			return nerr
		}
	}

	// get naples info from Netagent
	// Note: struct redefined here to avoid dependency on netagent package
	var naplesInfo struct {
		DSCMode     string   `json:"dsc-mode,omitempty"`
		DSCName     string   `json:"dsc-name,omitempty"`
		MgmtIP      string   `json:"mgmt-ip,omitempty"`
		Controllers []string `json:"controllers,omitempty"`
		IsConnectedToVenice bool `json:"is-connected-to-venice"`
	}

	status, err = agentClient.Req("GET", "https://"+nodeIP+":8888/api/mode/", nil, &naplesInfo)
	if err != nil || status != http.StatusOK {
		nerr := fmt.Errorf("Error checking netagent health. Status: %v, err: %v", status, err)
		log.Errorf("%v", nerr)
		return nerr
	}

	// Use type safe strings here. TODO
	if !strings.Contains(strings.ToLower(naplesInfo.DSCMode), "network") {
		nerr := fmt.Errorf("Naples/Netagent is in incorrect mode: %s", naplesInfo.DSCMode)
		log.Errorf("%v", nerr)
		return nerr
	} else if !naplesInfo.IsConnectedToVenice {
		nerr := fmt.Errorf("Netagent is not connected to Venice")
		log.Errorf("%v", nerr)
		return nerr
	}

	return nil
}
