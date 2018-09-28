package main

import (
	"context"
	"fmt"
	"testing"
	"time"

	agent "github.com/pensando/sw/iota/svcs/agent"
	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
	client "github.com/pensando/sw/iota/svcs/agent/grpc/client"
	server "github.com/pensando/sw/iota/svcs/agent/grpc/server"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestAgentServer(t *testing.T) {
	if true {
		return
	}
	go func() {
		main()
	}()

	time.Sleep(5 * time.Second)
	server.StopServer()
}

func TestBringUpTeardown(t *testing.T) {
	go func() {
		server.StartServer(9988)
	}()
	defer server.StopServer()
	time.Sleep(1 * time.Second)
	client, _ := client.NewAppAgentClient("localhost", 9988)
	TestUtils.Assert(t, client != nil, "Unable to connect server")
	resp, err := client.BringUp(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})
	TestUtils.Assert(t, err == nil && resp.Status == pb.ApiStatus_API_STATUS_OK,
		"Unable to bring up container")
	resp, err = client.Teardown(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})
	TestUtils.Assert(t, err == nil && resp.Status == pb.ApiStatus_API_STATUS_OK,
		"Unable to bring down container")
}

func TestRunCommand(t *testing.T) {
	go func() {
		server.StartServer(9988)
	}()
	defer server.StopServer()
	time.Sleep(1 * time.Second)
	client, _ := client.NewAppAgentClient("localhost", 9988)
	TestUtils.Assert(t, client != nil, "Unable to connect server")
	resp, err := client.BringUp(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})
	TestUtils.Assert(t, err == nil && resp.Status == pb.ApiStatus_API_STATUS_OK,
		"Unable to bring up container")
	defer client.Teardown(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})

	cmdResp, cmdErr := client.RunCommand(context.Background(), &pb.Command{Cmd: "ls",
		App: "agent-test"})
	fmt.Println(cmdResp)
	fmt.Println("Command out", cmdResp.GetStdout())
	TestUtils.Assert(t, cmdErr == nil && cmdResp.Status == pb.ApiStatus_API_STATUS_OK && cmdResp.RetCode == 0,
		"Running command failed")

	defer client.Teardown(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})
}

func TestAttachInterface(t *testing.T) {
	go func() {
		server.StartServer(9988)
	}()
	defer server.StopServer()
	time.Sleep(1 * time.Second)
	client, _ := client.NewAppAgentClient("localhost", 9988)
	TestUtils.Assert(t, client != nil, "Unable to connect server")
	resp, err := client.BringUp(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})
	TestUtils.Assert(t, err == nil && resp.Status == pb.ApiStatus_API_STATUS_OK,
		"Unable to bring up container")
	defer client.Teardown(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})

	intfResp, intfErr := client.AttachInterface(context.Background(),
		&pb.Interface{App: "agent-test", Name: "JUNK", MacAddress: "aa:bb:cc:dd:ee:ff"})
	TestUtils.Assert(t, intfErr == nil && intfResp.Status == pb.ApiStatus_API_STATUS_FAILED,
		"Attach Inteface API failed")

	defer client.Teardown(context.Background(),
		&pb.AppConfig{Name: "agent-test", Registry: "alpine"})
}

func init() {
	agent.LogDir = "/tmp/"
}
