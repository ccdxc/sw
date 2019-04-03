// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"flag"
	"fmt"
	"testing"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var testbedParams = flag.String("testbed", "", "testbed params file (i.e warmd.json)")
var mockTestbed = flag.Bool("mock", false, "Mock a venice testbed")

func newModel(t *testing.T) *SysModel {
	// create a testbed
	tb, err := NewTestBed("3Venice_3NaplesSim", *testbedParams)
	AssertOk(t, err, "Error creating testbed")

	err = tb.SetupConfig()
	AssertOk(t, err, "Error initing venice cluster")

	model, err := NewSysModel(tb)
	AssertOk(t, err, "Error creating sysmodel")

	// err = model.SetupDefaultConfig()
	// AssertOk(t, err, "Error setting up default config")

	return model
}

func TestSetupTestbed(t *testing.T) {
	if *testbedParams == "" {
		t.Skip("Skipping since no testbed is specified")
	}

	// create a testbed
	tb, err := NewTestBed("3Venice_3NaplesSim", *testbedParams)
	AssertOk(t, err, "Error creating testbed")

	err = tb.SetupConfig()
	AssertOk(t, err, "Error initing venice cluster")

	model, err := NewSysModel(tb)
	AssertOk(t, err, "Error creating sysmodel")

	err = model.SetupDefaultConfig(false)
	AssertOk(t, err, "Error setting up default config")
}

func TestRunVeniceTrigger(t *testing.T) {
	if *testbedParams == "" {
		t.Skip("Skipping since no testbed is specified")
	}

	model := newModel(t)
	Assert(t, (model != nil), "Failed to create a model")

	cmds := []*iota.Command{}

	for _, vn := range model.veniceNodes {
		log.Infof("Testing venice cmd on node: %v", vn.iotaNode.Name)
		cmd1 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("mkdir -p /pensando/iota/k8s/"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd1)

		cmd2 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("sudo cp -r /var/lib/pensando/pki/kubernetes/apiserver-client /pensando/iota/k8s/"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd2)

		cmd21 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("sudo chmod -R 777 /pensando/iota/k8s/"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd21)

		cmd3 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("if ! [ -f /tmp/kubernetes-server-linux-amd64.tar.gz ]; then curl -fL --retry 3 --keepalive-time 2  -o /tmp/kubernetes-server-linux-amd64.tar.gz  https://storage.googleapis.com/kubernetes-release/release/v1.7.14/kubernetes-server-linux-amd64.tar.gz; fi"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd3)

		cmd4 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("tar xvf /tmp/kubernetes-server-linux-amd64.tar.gz  kubernetes/server/bin/kubectl"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd4)

		cmd5 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("chmod 755 kubernetes/server/bin/kubectl"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd5)

		cmd6 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("sudo cp kubernetes/server/bin/kubectl /usr/local/bin/kubectl"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd6)

		cmd12 := iota.Command{
			Mode: iota.CommandMode_COMMAND_FOREGROUND,
			Command: fmt.Sprintf(`echo '/pensando/iota/kubernetes/server/bin/kubectl config set-cluster e2e --server=https://%s:6443 --certificate-authority=/pensando/iota/k8s/apiserver-client/ca-bundle.pem; 
			/pensando/iota/kubernetes/server/bin/kubectl config set-context e2e --cluster=e2e --user=admin;
			/pensando/iota/kubernetes/server/bin/kubectl config use-context e2e;
			/pensando/iota/kubernetes/server/bin/kubectl config set-credentials admin --client-certificate=/pensando/iota/k8s/apiserver-client/cert.pem --client-key=/pensando/iota/k8s/apiserver-client/key.pem;
			' > /pensando/iota/setup_kubectl.sh
			`, vn.iotaNode.Name),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd12)

		cmd13 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("chmod +x /pensando/iota/setup_kubectl.sh"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd13)

		cmd14 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("/pensando/iota/setup_kubectl.sh"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd14)
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_SERIAL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(model.tb.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
		t.Fatalf("Err: %v", err)
	}

	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode != 0 {
			t.Fatalf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}
	log.Infof("Got Trigger resp: %+v", triggerResp)

}

func TestStartKibana(t *testing.T) {
	if *testbedParams == "" {
		t.Skip("Skipping since no testbed is specified")
	}

	model := newModel(t)
	Assert(t, (model != nil), "Failed to create a model")

	cmds := []*iota.Command{}

	for _, vn := range model.veniceNodes {
		cmd1 := iota.Command{
			Mode: iota.CommandMode_COMMAND_FOREGROUND,
			Command: fmt.Sprintf(`echo 'docker run --rm --name kibana --network host \
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
			`, vn.iotaNode.Name),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd1)

		cmd2 := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("chmod +x /pensando/iota/start_kibana.sh"),
			EntityName: vn.iotaNode.Name + "_venice",
			NodeName:   vn.iotaNode.Name,
		}
		cmds = append(cmds, &cmd2)
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_SERIAL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(model.tb.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
		t.Fatalf("Err: %v", err)
	}

	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode != 0 {
			t.Fatalf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}
	log.Infof("Got Trigger resp: %+v", triggerResp)

}
