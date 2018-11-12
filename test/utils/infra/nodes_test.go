package infra

import (
	"fmt"
	"io"
	"log"
	"net"
	"net/http"
	"os"
	"testing"
	"time"

	"golang.org/x/crypto/ssh"
	context "golang.org/x/net/context"

	"google.golang.org/grpc"

	pb "github.com/pensando/sw/test/utils/infra/agent/grpc/api/pb"
	Common "github.com/pensando/sw/test/utils/infra/common"
	Helpers "github.com/pensando/sw/test/utils/infra/common"
	AgentConfig "github.com/pensando/sw/test/utils/infra/config"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

// AppServer is used to implement App server
type AppServer struct {
}

const (
	appNotFound = "App not running."
	arpTimeout  = 3 * time.Second
)

// BringUp Bring up app
func (s *AppServer) BringUp(ctx context.Context, in *pb.AppConfig) (*pb.AppStatus, error) {
	return &pb.AppStatus{Response: "", Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *AppServer) Teardown(ctx context.Context, in *pb.AppConfig) (*pb.AppStatus, error) {
	return &pb.AppStatus{Response: "", Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// AttachInterface attaches the specified to the container.
func (s *AppServer) AttachInterface(ctx context.Context, in *pb.Interface) (*pb.InterfaceStatus, error) {
	return &pb.InterfaceStatus{Response: "",
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// AttachInterface attaches the specified to the container.
func (s *AppServer) AddVlanInterface(ctx context.Context, in *pb.VlanInterface) (*pb.InterfaceStatus, error) {
	return &pb.InterfaceStatus{Response: "",
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *AppServer) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	return &pb.CommandStatus{
		Status: pb.ApiStatus_API_STATUS_OK,
	}, nil

}

//NewAppServer Returns an App server
func NewAppServer() *AppServer {
	return &AppServer{}
}

// NaplesSim is used to implement Naples Sim
type NaplesSim struct {
}

// BringUp Bring up app
func (s *NaplesSim) BringUp(ctx context.Context, in *pb.NaplesSimConfig) (*pb.NaplesStatus, error) {
	return &pb.NaplesStatus{Response: "", Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *NaplesSim) Teardown(ctx context.Context, in *pb.NaplesSimConfig) (*pb.NaplesStatus, error) {
	return &pb.NaplesStatus{Response: "", Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *NaplesSim) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	return &pb.CommandStatus{
		Status: pb.ApiStatus_API_STATUS_OK,
	}, nil
}

//ConfigureHntap Configure Hntap.
func (s *NaplesSim) ConfigureHntap(ctx context.Context, in *pb.HntapConfig) (*pb.NaplesStatus, error) {
	return &pb.NaplesStatus{Response: "Naples Sim running.",
		Status: pb.ApiStatus_API_STATUS_OK}, nil
}

//NewNaplesSim Returns Naples Sim context
func NewNaplesSim() *NaplesSim {
	return &NaplesSim{}
}

type Qemu struct{}

//BringUp bringUp Qemu environment
func (s *Qemu) BringUp(ctx context.Context, in *pb.QemuConfig) (*pb.QemuStatus, error) {
	return &pb.QemuStatus{Response: "Qemu bring up successful", Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *Qemu) Teardown(ctx context.Context, in *pb.QemuConfig) (*pb.QemuStatus, error) {
	return &pb.QemuStatus{Response: "Qemu teardowm up successful", Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *Qemu) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	return &pb.CommandStatus{
		Status:  pb.ApiStatus_API_STATUS_OK,
		RetCode: int32(0),
		Stdout:  "",
	}, nil
}

func NewQemu() *Qemu {
	return &Qemu{}
}

var grpcServer *grpc.Server

func StartServer(port int) {
	// create a listener on TCP port 7777
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	// create a gRPC server object
	grpcServer = grpc.NewServer()
	// attach the App service to the server
	pb.RegisterAppAgentServer(grpcServer, NewAppServer())
	pb.RegisterNaplesSimServer(grpcServer, NewNaplesSim())
	pb.RegisterQemuServer(grpcServer, NewQemu())
	// start the server
	log.Print("Starting Grpc server started on port :", port)
	if err := grpcServer.Serve(lis); err == grpc.ErrServerStopped {
		// grpc doesn't close our listener for us, so we have to do it ourselves.
		log.Print("Stopped Grpc server started on port :", port)
		lis.Close()
	}
}

//StopServer stops grpc server
func StopServer() {
	log.Print("Stopping grpc server")
	grpcServer.Stop()
}

var NoBody = noBody{}

type noBody struct{}

func (noBody) Read([]byte) (int, error)         { return 0, io.EOF }
func (noBody) Close() error                     { return nil }
func (noBody) WriteTo(io.Writer) (int64, error) { return 0, nil }

func TestNodesLoad(t *testing.T) {
	t.Skip("Test no longer needed post IOTA. TODO remove test code")
	Common.Run = Common.RunCmd
	Common.SudoCmd = func(cmd string) string {
		return cmd
	}

	SrcNaplesDirectory = "./test_cfg"

	vmUserName = "root"
	vmPassword = "root"
	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "testnode", "sickp/alpine-sshd:7.5-r2"}
	Helpers.Run(startCmd, 0, false, false, nil)
	stopCmd := []string{"docker", "stop", "testnode"}
	defer Helpers.Run(stopCmd, 0, false, false, nil)
	fmt.Println("Started Server")
	go StartServer(agentPort)
	defer StopServer()

	fmt.Println(os.Getwd())
	userTopoFile := "./test_cfg/user_topo.yaml"
	warmdFile := "./test_cfg/warmd.json"
	userTopo := loadUserTopo(userTopoFile)

	TestUtils.Assert(t, userTopo != nil, "User topo load failed!")
	TestUtils.Assert(t, len(userTopo.Nodes) == 1, "Length of nodes mismatch")
	TestUtils.Assert(t, len(userTopo.Apps) == 2, "Length of nodes mismatch")
	userTopo.validate()
	LogDir = "/tmp/"
	linuxBuildEnv = []string{}
	ctx, err := NewInfraCtx(userTopoFile, warmdFile, false)
	if err != nil {
		fmt.Println(err.Error())
	}
	TestUtils.Assert(t, err == nil, "Infra Ctx creation failed")
	intfractx, _ := ctx.(*infraCtx)
	//10 Entities: Each node has 5 (VM, naples, switch, 2 apps)
	TestUtils.Assert(t, len(intfractx.res) == 4, "Number of remote entities mismatched!")
	TestUtils.Assert(t, intfractx.res["node1"] != nil, "Node 1 present!")
	vmentity := intfractx.res["node1"].(*vmEntity)
	TestUtils.Assert(t, vmentity.kind == EntityKindVM, "Node 1 is not VM!")
	vmEntities := intfractx.FindRemoteEntity(EntityKindVM)
	TestUtils.Assert(t, len(vmEntities) == 1, "Number of Vm entities don't match")
	naples := intfractx.FindRemoteEntity(EntityKindNaples)
	TestUtils.Assert(t, len(naples) == 1, "Number of naples entities don't match")
	apps := intfractx.FindRemoteEntity(EntityKindContainer)
	TestUtils.Assert(t, len(apps) == 2, "Number of app entities don't match")
	switch v := naples[0].Parent().(type) {
	case *vmEntity:
		TestUtils.Assert(t, v.ipAddress != "", "IP address not set")
	default:
		TestUtils.Assert(t, false, "Parent type failed!")
	}
	switch v := apps[0].Parent().(type) {
	case *vmEntity:
		print(v)
	default:
		TestUtils.Assert(t, false, "Parent type failed!")
	}
	retCode, _, _ := apps[0].(*appEntity).Exec("ls", false, false)
	apps[0].(*appEntity).ScpTo("src", "dest")
	TestUtils.Assert(t, retCode == 0, "Retcode not zero")
	TestUtils.Assert(t, len(ctx.(*infraCtx).getHostApps(vmentity)) == 2, "Number of host apps don't match")
	AgentConfig.DoConfig = func(req *http.Request) (*http.Response, error) {
		return &http.Response{Body: noBody{}}, nil
	}
	deviceJSONFile := os.Getenv("GOPATH") + "/src/github.com/pensando/sw/platform/src/app/nicmgrd/etc/eth-smart.json"
	ctx.DoConfiguration(deviceJSONFile)
	ctx.PrintTopology()
	ctx.(*infraCtx).CleanUp()
	jsonPath = "./test_cfg/warmd.json"
	Helpers.Run(startCmd, 0, false, false, nil)
	RunCmd("ls")
	client, _ := ssh.Dial("tcp", fmt.Sprintf("127.0.0.1:22"), &ssh.ClientConfig{
		User:            "root",
		Auth:            []ssh.AuthMethod{ssh.Password("root")},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	})
	ctx.FindRemoteEntityByName("node1")
	runSSHWithClient(client, "ls")
	CopyLogs([]string{"./test_cfg/warmd.json"}, "/tmp")
	copyLogsWithSSHClient(client, []string{"./test_cfg/warmd.json"}, "/tmp")
}

func TestQemuNodesLoad(t *testing.T) {
	t.Skip("Test no longer needed post IOTA. TODO remove test code")
	Common.Run = Common.RunCmd
	Common.SudoCmd = func(cmd string) string {
		return cmd
	}

	SrcNaplesDirectory = "./test_cfg"

	vmUserName = "root"
	vmPassword = "root"
	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "testnode", "sickp/alpine-sshd:7.5-r2"}
	Helpers.Run(startCmd, 0, false, false, nil)
	stopCmd := []string{"docker", "stop", "testnode"}
	defer Helpers.Run(stopCmd, 0, false, false, nil)
	fmt.Println("Started Server")
	go StartServer(agentPort)
	defer StopServer()

	fmt.Println(os.Getwd())
	userTopoFile := "./test_cfg/user_topo_qemu.yaml"
	warmdFile := "./test_cfg/warmd.json"
	userTopo := loadUserTopo(userTopoFile)

	TestUtils.Assert(t, userTopo != nil, "User topo load failed!")
	TestUtils.Assert(t, len(userTopo.Nodes) == 1, "Length of nodes mismatch")
	TestUtils.Assert(t, len(userTopo.Apps) == 2, "Length of nodes mismatch")
	userTopo.validate()
	LogDir = "/tmp/"
	linuxBuildEnv = []string{}
	localPlatfromSrc = SrcNaplesDirectory
	ctx, err := NewInfraCtx(userTopoFile, warmdFile, false)
	if err != nil {
		fmt.Println(err.Error())
	}
	TestUtils.Assert(t, err == nil, "Infra Ctx creation failed")
	intfractx, _ := ctx.(*infraCtx)
	//10 Entities: Each node has 5 (VM, naples, switch, 2 apps)
	TestUtils.Assert(t, len(intfractx.res) == 5, "Number of remote entities mismatched!")
	TestUtils.Assert(t, intfractx.res["node1"] != nil, "Node 1 present!")
	vmentity := intfractx.res["node1"].(*vmEntity)
	TestUtils.Assert(t, vmentity.kind == EntityKindVM, "Node 1 is not VM!")
	vmEntities := intfractx.FindRemoteEntity(EntityKindVM)
	TestUtils.Assert(t, len(vmEntities) == 1, "Number of Vm entities don't match")
	qemuEntities := intfractx.FindRemoteEntity(EntityKindQemu)
	TestUtils.Assert(t, len(qemuEntities) == 1, "Number of Qemu entities don't match")
	naples := intfractx.FindRemoteEntity(EntityKindNaples)
	TestUtils.Assert(t, len(naples) == 1, "Number of naples entities don't match")
	apps := intfractx.FindRemoteEntity(EntityKindContainer)
	TestUtils.Assert(t, len(apps) == 2, "Number of app entities don't match")
	switch v := naples[0].Parent().(type) {
	case *vmEntity:
		TestUtils.Assert(t, v.ipAddress != "", "IP address not set")
	default:
		TestUtils.Assert(t, false, "Parent type failed!")
	}
	switch v := apps[0].Parent().(type) {
	case *vmEntity:
		print(v)
	default:
		TestUtils.Assert(t, false, "Parent type failed!")
	}
	retCode, _, _ := apps[0].(*appEntity).Exec("ls", false, false)
	apps[0].(*appEntity).ScpTo("src", "dest")
	TestUtils.Assert(t, retCode == 0, "Retcode not zero")
	TestUtils.Assert(t, len(ctx.(*infraCtx).getHostApps(vmentity)) == 2, "Number of host apps don't match")
	AgentConfig.DoConfig = func(req *http.Request) (*http.Response, error) {
		return &http.Response{Body: noBody{}}, nil
	}
	deviceJSONFile := os.Getenv("GOPATH") + "/src/github.com/pensando/sw/platform/src/app/nicmgrd/etc/eth-smart.json"
	ctx.DoConfiguration(deviceJSONFile)
	ctx.PrintTopology()
	ctx.(*infraCtx).CleanUp()
	jsonPath = "./test_cfg/warmd.json"
	Helpers.Run(startCmd, 0, false, false, nil)
	RunCmd("ls")
	client, _ := ssh.Dial("tcp", fmt.Sprintf("127.0.0.1:22"), &ssh.ClientConfig{
		User:            "root",
		Auth:            []ssh.AuthMethod{ssh.Password("root")},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	})
	ctx.FindRemoteEntityByName("node1")
	runSSHWithClient(client, "ls")
	CopyLogs([]string{"./test_cfg/warmd.json"}, "/tmp")
	copyLogsWithSSHClient(client, []string{"./test_cfg/warmd.json"}, "/tmp")
}

func TestConfigStub(t *testing.T) {
	configBuffer := NewConfigBuffer()
	configBuffer.FindObject("kind")
	configBuffer.Load("config")
	configBuffer.LoadFile("configPath")
	configBuffer.Merge("config")
	configBuffer.MergeFile("configPath")
	configBuffer.SaveObject("obj")
	configBuffer.GenerateObject("kind", 1)
	configBuffer.GenerateConfig("kind", 1)
}
