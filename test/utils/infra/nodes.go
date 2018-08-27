package infra

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"
	yaml "gopkg.in/yaml.v2"

	Agent "github.com/pensando/sw/test/utils/infra/agent"
	NodeService "github.com/pensando/sw/test/utils/infra/agent/service"
	Helpers "github.com/pensando/sw/test/utils/infra/common"
	cfg "github.com/pensando/sw/test/utils/infra/config"
)

var agentSrc string
var naplesAgentCfgPath string

const (
	e2eRegistry = "registry.test.pensando.io:5000/pensando/nic/e2e:2.0"
)

var (
	vmUserName      = "vm"
	vmPassword      = "vm"
	vmDataInterface = "eth1"
	tunnelIPSubnet  = "192.168.10.0/24"
	tunnelIPStart   = "192.168.10.11"
)

var sudoCmd = func(cmd string) string {
	return "sudo " + cmd
}

type userTopo struct {
	Nodes []struct {
		Node struct {
			Name string `yaml:"name"`
			Kind string `yaml:"kind"`
		} `yaml:"node"`
	} `yaml:"nodes"`
	Apps []struct {
		App struct {
			Name   string `yaml:"name"`
			Kind   string `yaml:"kind"`
			Parent string `yaml:"parent"`
		} `yaml:"app"`
	} `yaml:"apps"`
}

func loadUserTopo(file string) *userTopo {
	yamlFile, err := os.Open(file)
	defer yamlFile.Close()
	if err != nil {
		log.Fatalln("Unable to open User topo file :", file)
	}

	byteValue, _ := ioutil.ReadAll(yamlFile)
	var usertopo userTopo
	yaml.Unmarshal(byteValue, &usertopo)
	return &usertopo
}

func (ut *userTopo) validate() {
	parentNodes := make(map[string]bool)
	for _, node := range ut.Nodes {
		if _, ok := parentNodes[node.Node.Name]; ok {
			log.Fatalf("Node name %s already used in the topo", node.Node.Name)
		} else {
			parentNodes[node.Node.Name] = true
		}
		switch node.Node.Kind {
		case DataNode:
		case ControlNode:
		default:
			log.Fatalf("Invalid Node type : %s", node.Node.Kind)
		}
	}

	appNodes := make(map[string]bool)
	for _, app := range ut.Apps {
		if _, ok := appNodes[app.App.Name]; ok {
			log.Fatalf("App name %s already used in the topo", app.App.Name)
		} else {
			appNodes[app.App.Name] = true
		}
		switch app.App.Kind {
		case ContainerType:
		default:
			print(app.App.Name, app.App.Parent)
			log.Fatalf("Invalid App type %s", app.App.Kind)
		}
		if _, ok := parentNodes[app.App.Parent]; !ok {
			log.Fatalf("Parent %s not found", app.App.Parent)
		}
	}
}

//vmEntity vm enttity
type vmEntity struct {
	remoteEntity
	nodeID     int
	userName   string
	passwd     string
	SftpHandle *sftp.Client
	SSHHandle  *ssh.Client
	services   *NodeService.NodeServices
}

//naplesEntity Naples Entity
type naplesEntity struct {
	remoteEntity
}

//appEntity Naples Entity
type appEntity struct {
	remoteEntity
}

func (vm *vmEntity) setupConnection() error {
	var err error
	vm.SftpHandle, err =
		Helpers.NewSFTPClient(vm.ipAddress,
			vm.userName, vm.passwd)
	if err != nil {
		err = errors.Wrapf(err, "SFTP connect failed")
		vm.log(err.Error())
		return err
	}
	config := &ssh.ClientConfig{
		User: vm.userName,
		Auth: []ssh.AuthMethod{
			ssh.Password(vm.passwd),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	if conn, err := ssh.Dial("tcp", vm.ipAddress+":22", config); err == nil {
		vm.SSHHandle = conn
	} else {
		err = errors.Wrapf(err, "SSH connect failed")
		vm.log(err.Error())
		return err
	}

	return nil
}

func (vm *vmEntity) teardownConnection() error {

	vm.log("Tearing down connection.")
	if vm.SftpHandle != nil {
		vm.SftpHandle.Close()
	}
	if vm.SSHHandle != nil {
		vm.SSHHandle.Close()
	}
	return nil
}

func (infraCtx *infraCtx) initDataNode(name string) {
	vmentity := vmEntity{
		remoteEntity: remoteEntity{name: name, kind: EntityKindVM}}
	infraCtx.res[name] = &vmentity
	naplesName := "Naples_" + name
	naplesentity := naplesEntity{remoteEntity{name: naplesName, kind: EntityKindNaples}}
	infraCtx.res[naplesName] = &naplesentity
	naplesentity.parent = &vmentity
}

func (infraCtx *infraCtx) initApp(name string, parent string) {
	appentity := appEntity{remoteEntity{name: name, kind: EntityKindContainer}}
	if parentEntity, ok := infraCtx.res[parent]; ok {
		appentity.parent = parentEntity
		infraCtx.res[name] = &appentity
	} else {
		log.Fatalln("Parent Node not found!")
	}
}
func (infraCtx *infraCtx) warmdInit(warmdFile string) error {

	file, e := ioutil.ReadFile(warmdFile)
	if e != nil {
		infraCtx.logger.Printf("Error opening Warmd Json : error: %v\n", e)
		return e
	}

	var warmd interface{}
	var err error
	err = json.Unmarshal(file, &warmd)
	if err != nil {
		log.Fatal(err)
		return err
	}

	jsonWarmd := warmd.(map[string]interface{})
	instances := jsonWarmd["Instances"]
	warmdInstances := instances.(map[string]interface{})

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)

	if len(vmEntities) > len(warmdInstances) {
		err := fmt.Errorf("User topo has more vm Entities %d than wamrd %d", len(vmEntities),
			len(warmdInstances))
		infraCtx.logger.Printf(err.Error())
		return err
	}

	cnt := 0
	for _, y := range warmdInstances {
		vm, ok := vmEntities[cnt].(*vmEntity)
		if !ok {
			return fmt.Errorf("invalid vm entity %+v", vmEntities[cnt])
		}
		vm.ipAddress = y.(string)
		vm.nodeID = cnt + 1
		//This should come from Warmd after all.
		vm.userName = vmUserName
		vm.passwd = vmPassword
		err := vm.setupConnection()
		if err != nil {
			err = errors.Wrapf(err,
				"Connection setup failed for VM : %s %v", vm.Name(), err)
			infraCtx.logger.Println(err)
			return err
		}
		cnt++
		if cnt == len(vmEntities) {
			break
		}
	}

	return nil
}

//LogWriter Helper to write to stdout and file.
type LogWriter log.Logger

func (w *LogWriter) Write(b []byte) (int, error) {
	(*log.Logger)(w).Print(string(b))
	return len(b), nil
}

func (vm *vmEntity) ScpTo(src, dest string) error {

	return nil
}

func (vm *vmEntity) Exec(cmd string, sudo bool, bg bool) (retCode int, stdout, stderr []string) {
	fmt.Println(cmd)
	var stdoutBuf, stderrBuf bytes.Buffer
	sshSession, err := vm.SSHHandle.NewSession()
	if err != nil {
		vm.log("Ssh handle creation failed, is node down ?")
		return -1, nil, nil
	}

	//remoteEntity.()
	modes := ssh.TerminalModes{
		ssh.ECHO:          0,     // disable echoing
		ssh.TTY_OP_ISPEED: 14400, // input speed = 14.4kbaud
		ssh.TTY_OP_OSPEED: 14400, // output speed = 14.4kbaud
	}

	if err = sshSession.RequestPty("xterm", 80, 40, modes); err != nil {
		vm.log("Ssh Pty session creation failed")
		return -1, nil, nil
	}

	sshOut, err := sshSession.StdoutPipe()
	if err != nil {
		vm.log("Unable to get stodut from ssh session")
		return -1, nil, nil
	}
	sshErr, err := sshSession.StderrPipe()
	if err != nil {
		vm.log("Unable to get stderr from ssh session")
		return -1, nil, nil
	}

	shout := io.MultiWriter(&stdoutBuf, (*LogWriter)(vm.logger))
	ssherr := io.MultiWriter(&stderrBuf, (*LogWriter)(vm.logger))

	go func() {
		if true {
			io.Copy(shout, sshOut)
		}
	}()
	go func() {
		if true {

			io.Copy(ssherr, sshErr)
		}
	}()

	if bg {
		cmd = "nohup sh -c  \"" + cmd + " 2>&1 >/dev/null </dev/null & \""
	} else {
		cmd = "sh -c \"" + cmd + "\""
	}

	if sudo {
		cmd = sudoCmd(cmd)
	}

	vm.log("Running command : " + cmd)
	if err = sshSession.Run(cmd); err != nil {
		vm.log("failed command : " + cmd)
		switch v := err.(type) {
		case *ssh.ExitError:
			retCode = v.Waitmsg.ExitStatus()
		default:
			retCode = -1
		}
	} else {
		vm.log("sucess command : " + cmd)
		retCode = 0
	}

	stdout = strings.Split(stdoutBuf.String(), "\n")
	stderr = strings.Split(stderrBuf.String(), "\n")
	vm.log(stdout)
	vm.log(stderr)
	vm.log("Return code : " + strconv.Itoa(retCode))

	return retCode, stdout, stderr

}

// bring up methond for naples
func (naples *naplesEntity) bringUp(ctx context.Context, errChannel chan error) {
	var err error
	var hostNode RemoteEntity

	hostNode, err = naples.GetHostingNode()
	if err != nil {
		/* PANIC as something weird we did in parsing topo file */
		panic("Unable to find Naples Hosting node for :" + naples.Name())
	}

	naples.log("Doing naples bring up on Node")
	/* First copy the Naples Artificats */
	err = Helpers.MultiSFTP(SrcNaplesDirectory, RemoteNaplesDirectory,
		[]*sftp.Client{hostNode.(*vmEntity).SftpHandle})

	if err != nil {
		err = errors.Wrapf(err, "Error in copying Naples files to VM : %s",
			hostNode.Name())
		naples.log(err.Error())
		errChannel <- err
		return
	}

	sshRunError := make(chan error, 0)
	go func() {
		/* TODO : Have to pass on num nodes if more than 2 */
		nodeid := hostNode.(*vmEntity).nodeID
		tunnelIPAddr, _ := Helpers.IncrementIP(tunnelIPStart, tunnelIPSubnet, byte(nodeid-1))
		err := hostNode.(*vmEntity).services.Naples.BringUp(ctx, &NodeService.NaplesSimConfig{
			Name: NaplesSimName, NodeID: uint32(nodeid),
			CtrlNwIPRange:   "11.1.1." + strconv.Itoa(32+32*nodeid) + "/27",
			TunnelIPStart:   tunnelIPStart,
			TunnelInterface: vmDataInterface, //TODO!!
			TunnelIPAddress: tunnelIPAddr,
		})
		if err != nil {
			sshRunError <- errors.Wrap(err, "Naples bring up failed.")
			return
		}
		time.Sleep(NaplesAgentBringUpDelay)
		sshRunError <- nil
	}()

	select {
	// we received the signal of cancelation in this channel
	case err := <-sshRunError:
		if err != nil {
			log.Print("Naples bring up failed on : " + hostNode.Name())
			errChannel <- err
		} else {
			log.Print("Naples bring up sucessfull : " + hostNode.Name())
		}
		break
	case <-ctx.Done():
		fmt.Println("Timeout : Canceling Naples bring up.")
		errChannel <- fmt.Errorf("Naples bring up timeout on : %s", hostNode.Name())
	}

	errChannel <- nil
}

func (appNode *appEntity) bringUp(ctx context.Context, errChannel chan error) {
	var err error
	var hostNode RemoteEntity

	appNode.log("Bringing up App :" + appNode.Name())
	hostNode, err = appNode.GetHostingNode()
	if err != nil {
		panic("Unable to find Naples Hosting node for :" + appNode.Name())
	}

	err = hostNode.(*vmEntity).services.App.BringUp(ctx,
		&NodeService.AppConfig{Name: appNode.Name(), Registry: e2eRegistry})

	if err != nil {
		errChannel <- errors.Wrap(err, "Agent bring up api Failed on App")
		return
	}

	appNode.log("Successfull bring up of App :" + appNode.Name())
	errChannel <- err
}

func (appNode *appEntity) configure(ep cfg.Endpoint, config *cfg.E2eCfg) error {
	getNetworkFromConfig := func(nwName string, fullCfg *cfg.E2eCfg) *cfg.Network {
		for _, network := range fullCfg.NetworksInfo.Networks {
			if network.NetworkMeta.Name == nwName {
				return &network
			}
		}

		return nil
	}

	network := getNetworkFromConfig(ep.EndpointSpec.NetworkName, config)
	if network == nil {
		log.Fatalln("Network not found in config!", ep.EndpointSpec.NetworkName)
	}

	hostNode, _ := appNode.GetHostingNode()

	appNode.ipAddress = strings.Split(ep.EndpointSpec.Ipv4Address, "/")[0]
	prefixLen, _ := strconv.Atoi(strings.Split(network.NetworkSpec.Ipv4Subnet, "/")[1])
	err := hostNode.(*vmEntity).services.App.AttachInterface(context.Background(),
		appNode.Name(),
		&NodeService.AppInterface{Name: ep.EndpointSpec.Interface,
			MacAddress: ep.EndpointSpec.MacAddresss,
			Vlan:       uint32(ep.EndpointSpec.UsegVlan),
			IPaddress:  appNode.ipAddress,
			PrefixLen:  uint32(prefixLen)})

	if err != nil {
		return errors.Wrap(err, "Attach Interface failed")
	}

	return nil
}

func (appNode *appEntity) ScpTo(src, dest string) error {

	dest = "/home" + "/" + appNode.Name() + "/" + dest

	hostNode, _ := appNode.GetHostingNode()

	if hostNode == nil {
		return errors.New("Hosting node not found")
	}

	return hostNode.ScpTo(src, dest)
}

func (appNode *appEntity) Exec(cmd string, sudo bool, bg bool) (retCode int, stdout, stderr []string) {

	hostNode, _ := appNode.GetHostingNode()

	if hostNode == nil {
		appNode.log("Hosting node not found!")
		return -1, nil, nil
	}

	var err error
	stdout, stderr, retCode, err = hostNode.(*vmEntity).services.App.RunCommand(context.Background(),
		appNode.Name(), cmd, bg)
	if err != nil {
		appNode.log("Error running command on the node" + err.Error())
	}

	return retCode, stdout, stderr
}

func (*vmEntity) bringUp(ctx context.Context, errChannel chan error) {
	errChannel <- nil
}

func (vm *vmEntity) teardown(ctx context.Context, errChannel chan error) {
	vm.teardownConnection()
	errChannel <- nil
}

func (appNode *appEntity) teardown(ctx context.Context, errChannel chan error) {
	var err error
	var hostNode RemoteEntity

	hostNode, err = appNode.GetHostingNode()
	if err != nil {
		panic("Unable to find Naples Hosting node for :" + appNode.Name())
	}

	err = hostNode.(*vmEntity).services.App.Teardown(ctx,
		&NodeService.AppConfig{Name: appNode.Name(), Registry: e2eRegistry})

	if err != nil {
		errChannel <- errors.Wrap(err, "Agent teardown api Failed on App")
		return
	}

	errChannel <- err
}

//GetIPAddress Get IP address of the app Node.
func (appNode *appEntity) GetIPAddress() (string, error) {
	return appNode.ipAddress, nil
}

func (naples *naplesEntity) teardown(ctx context.Context, errChannel chan error) {
	naples.log("Bringing down naples entity")
	vm, _ := naples.GetHostingNode()
	vm.(*vmEntity).Exec("docker stop naples-sim", true, false)
	errChannel <- nil
}

func (infraCtx *infraCtx) initVMEntities() error {

	buildAgent := func() {
		cwd, _ := os.Getwd()
		os.Chdir(agentSrc)
		defer os.Chdir(cwd)
		cmd := []string{"go", "build", "-o", RemoteInfraDirectory + "/" + agentExecName, "."}
		env := []string{"GOOS=linux", "GOARCH=amd64"}
		if _, stdout, err := Helpers.Run(cmd, 0, false, false, env); err != nil {
			panic("Building agent failed!" + stdout)
		}
		cmd = []string{"chmod", "+x", RemoteInfraDirectory + "/" + agentExecName}
		if _, stdout, err := Helpers.Run(cmd, 0, false, false, env); err != nil {
			panic("Chmod failed on agent!" + stdout)
		}
	}
	buildAgent()
	setupAgents := func() error {
		vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)
		for _, vm := range vmEntities {
			cmdStart := "mkdir -p " + RunnerDirectory + " && chmod 777  " + RunnerDirectory
			ret, _, _ := vm.(*vmEntity).Exec(cmdStart, true, false)
			if ret != 0 {
				return errors.Errorf("Failed to create run directory on %s", vm.Name())
			}
			err := Helpers.MultiSFTP(RemoteInfraDirectory, RunnerDirectory,
				[]*sftp.Client{vm.(*vmEntity).SftpHandle})
			if err != nil {
				return errors.Wrap(err, "Copying agent binay failed on "+vm.Name())
			}
			cmdStart = RemoteInfraDirectory + "/" + agentExecName + " -port=" + strconv.Itoa(agentPort)
			fmt.Println("Executing cmd ", cmdStart)
			ret, _, _ = vm.(*vmEntity).Exec(cmdStart, true, true)
			if ret != 0 {
				return errors.Errorf("Agent bring up failed on node : %s", vm.Name())
			}
			time.Sleep(1 * time.Second)
			fmt.Println("Connecting to Agent..")

			if vm.(*vmEntity).services, err = Agent.InitNodeService(vm.(*vmEntity).ipAddress, agentPort); err != nil {
				return errors.Wrap(err, "Error in Initializing node services : "+vm.(*vmEntity).ipAddress+":"+strconv.Itoa(agentPort))

			}
			log.Println("Agent bring up successful on node : " + vm.(*vmEntity).ipAddress + ":" + strconv.Itoa(agentPort))
		}
		return nil
	}
	buildAgent()
	return setupAgents()
}

func (infraCtx *infraCtx) verifyVMConnectivity() error {

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)
	dumpIfconfig := func() {
		for _, vm := range vmEntities {
			ifconfigCmd := "ifconfig " + vmDataInterface
			_, stdout, _ := vm.(*vmEntity).Exec(ifconfigCmd, true, false)
			infraCtx.logger.Println(stdout)
		}

	}

	if len(vmEntities) < 2 {
		dumpIfconfig()
		infraCtx.logger.Println("Number of nodes less 2 , skipping connectivity check.")
		return nil
	}
	for _, vm := range vmEntities {
		for i := 0; i < len(vmEntities); i++ {
			//TODO : Change from hardcoded values
			nextIP, _ := Helpers.IncrementIP(tunnelIPStart, tunnelIPSubnet, byte(i))
			pingCmd := "ping -c 5 " + nextIP
			ret, stdout, _ := vm.(*vmEntity).Exec(pingCmd, false, false)
			infraCtx.logger.Println(stdout)
			if ret != 0 {
				dumpIfconfig()
				return errors.Errorf("Ping to destination %s failed from :", nextIP, vm.Name())
			}
		}
	}
	infraCtx.logger.Println("Data Path connectivity verfied")
	return nil
}

func (infraCtx *infraCtx) resetVMEntities() error {

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)
	for _, vm := range vmEntities {
		cmd := "pkill -9 " + agentExecName
		ret, _, _ := vm.(*vmEntity).Exec(cmd, true, false)
		if ret != 0 {
			infraCtx.logger.Println("Killing agent successful on node : " + vm.(*vmEntity).ipAddress + ":" + strconv.Itoa(agentPort))
		} else {
			infraCtx.logger.Println("Killing agent failed on node : " + vm.(*vmEntity).ipAddress + ":" + strconv.Itoa(agentPort))
		}
		cmd = "rm -rf " + RunnerDirectory
		fmt.Println("Running cmd..")
		vm.(*vmEntity).Exec(cmd, true, false)

		// Kill all apps for now
		cmd = "docker ps -q --filter ancestor=" + e2eRegistry + "  | xargs -r docker stop"
		vm.(*vmEntity).Exec(cmd, true, false)
		if ret != 0 {
			infraCtx.logger.Println("Killing all apps on node : " + vm.(*vmEntity).ipAddress + ":" + strconv.Itoa(agentPort))
		} else {
			infraCtx.logger.Println("Killing all apps failed on node : " + vm.(*vmEntity).ipAddress + ":" + strconv.Itoa(agentPort))
		}
	}
	/* Not really blocking, ignore any errors */
	return nil
}

func (infraCtx *infraCtx) bringUp() error {

	infraCtx.resetVMEntities()

	/* First create infra directory to copy all the stuff here */
	_ = os.Mkdir(RemoteInfraDirectory, 0777)

	if err := infraCtx.initVMEntities(); err != nil {
		err = errors.Wrap(err, "Vm entity init failed!")
		infraCtx.logger.Println(err.Error())
		return err
	}

	doBringUp := func(entities []RemoteEntity) error {
		errChannel := make(chan error, len(entities))
		for _, entity := range entities {
			ctx, cancel := context.WithTimeout(context.Background(), NodeBringUpTimeout)
			defer cancel()
			go entity.bringUp(ctx, errChannel)
		}
		for i := 0; i < len(entities); i++ {
			if err := <-errChannel; err != nil {
				infraCtx.logger.Println("Error in bringup " + err.Error())
				return err
			}
		}
		return nil
	}

	if err := doBringUp(infraCtx.FindRemoteEntity(EntityKindNaples)); err != nil {
		return err
	}

	if err := doBringUp(infraCtx.FindRemoteEntity(EntityKindContainer)); err != nil {
		return err
	}

	/* Finally verify Vm Connectivity */
	if err := infraCtx.verifyVMConnectivity(); err != nil {
		return errors.Wrap(err, "VM connectivity verification failed!")
	}
	return nil
}

func (infraCtx *infraCtx) CleanUp() error {

	wg := sync.WaitGroup{}
	entities := infraCtx.FindRemoteEntity("")
	errChannel := make(chan error, len(entities))
	for _, entity := range entities {
		ctx, cancel := context.WithTimeout(context.Background(), NodeTeardownTimeout)
		defer cancel()
		fmt.Println("Teardown ", entity.Name())
		go entity.teardown(ctx, errChannel)
	}
	wg.Wait()

	for i := 0; i < len(entities); i++ {
		if err := <-errChannel; err != nil {
			/* Report and continue */
			log.Printf("Error in teardown %v\n", err)
		}
	}

	infraCtx.resetVMEntities()

	_ = os.RemoveAll(RemoteInfraDirectory)
	return nil
}

func (infraCtx *infraCtx) setupLogger() {

	/* Create log directory to dump all infra logs */
	_ = os.Mkdir(LogDir, 644)

	file, err := os.OpenFile(LogDir+"/"+InfraLogFile, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
	if err != nil {
		log.Fatalln("Failed to open log file", InfraLogFile, ":", err)
	}
	multi := io.MultiWriter(file, os.Stdout)
	infraCtx.logger = log.New(multi, "TEST-INFRA: ",
		log.Ldate|log.Ltime|log.Lshortfile)

}

// NewInfraCtx initializes/returns an instance of the global context
func NewInfraCtx(topoFile string, warmdFile string) (Context, error) {

	ut := loadUserTopo(topoFile)
	ut.validate()

	infraCtx := &infraCtx{}
	infraCtx.res = make(map[string]RemoteEntity)
	infraCtx.setupLogger()
	for _, node := range ut.Nodes {
		switch node.Node.Kind {
		case DataNode:
			infraCtx.initDataNode(node.Node.Name)
		case ControlNode:
		default:
			log.Fatalf("Invalid Node type : %s", node.Node.Kind)
		}
	}

	if err := infraCtx.warmdInit(warmdFile); err != nil {
		return nil, errors.Wrap(err, "Warmd Association failed!")
	}

	for _, app := range ut.Apps {
		infraCtx.initApp(app.App.Name, app.App.Parent)
	}

	if err := infraCtx.bringUp(); err != nil {
		return nil, err
	}

	return infraCtx, nil
}

func init() {
	// Harcoding to GRPC agent type for now.
	agentSrc = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/utils/infra/agent/grpc/"
	naplesAgentCfgPath = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/utils/infra/test_cfg"
}

func (infraCtx *infraCtx) getHostApps(host RemoteEntity) []RemoteEntity {
	appEntities := infraCtx.FindRemoteEntity(EntityKindContainer)
	hostAppEntities := []RemoteEntity{}
	for _, appEntity := range appEntities {
		if appHost, err := appEntity.GetHostingNode(); err == nil {
			if appHost == host {
				hostAppEntities = append(hostAppEntities, appEntity)
			}
		}
	}
	return hostAppEntities
}
