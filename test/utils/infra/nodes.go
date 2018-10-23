package infra

import (
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

	"github.com/pensando/test-infra/public"

	ionic "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	Agent "github.com/pensando/sw/test/utils/infra/agent"
	NodeService "github.com/pensando/sw/test/utils/infra/agent/service"
	Helpers "github.com/pensando/sw/test/utils/infra/common"
	cfg "github.com/pensando/sw/test/utils/infra/config"
)

var agentSrc string
var naplesAgentCfgPath string
var ethSmartJSON string

const (
	e2eRegistry = "registry.test.pensando.io:5000/pensando/nic/e2e:2.0"
	qemuImage   = "build-16"
)

var (
	vmUserName             = "vm"
	vmPassword             = "vm"
	vmDataInterface        = "eth1"
	tunnelIPSubnet         = "192.168.10.0/24"
	tunnelIPStart          = "192.168.10.11"
	linuxBuildEnv          = []string{"GOOS=linux", "GOARCH=amd64"}
	tunneledModeIntfs      = [...]string{"eth2"}
	passedThroughModeIntfs = [...]string{"eth1", "eth2"}
)

var (
	//SrcNaplesDirectory Naples image source directory
	SrcNaplesDirectory = "/sw/nic/obj/images"
	localPlatfromSrc   = "/sw/platform"

	remoteSrc         = "/home/vm/sw"
	remotePlatformSrc = remoteSrc + "/platform"
)

var sudoCmd = func(cmd string) string {
	return "sudo " + cmd
}

type userTopo struct {
	Nodes []struct {
		Node struct {
			Name string `yaml:"name"`
			Kind string `yaml:"kind"`
			Qemu bool   `yaml:"qemu"`
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
	hasQemu    bool
	SftpHandle *sftp.Client
	SSHHandle  *ssh.Client
	services   *NodeService.NodeServices
}

//naplesEntity Naples Entity
type naplesEntity struct {
	remoteEntity
	withQemu        bool
	passThroughMode bool
}

//appEntity Naples Entity
type appEntity struct {
	onQemu bool
	remoteEntity
}

//appEntity Naples Entity
type qemuEntity struct {
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

func (infraCtx *infraCtx) initDataNode(name string, qemu bool, passThroughMode bool) {
	vmentity := vmEntity{
		remoteEntity: remoteEntity{name: name, kind: EntityKindVM}}
	infraCtx.res[name] = &vmentity
	naplesName := "Naples_" + name
	naplesentity := naplesEntity{remoteEntity: remoteEntity{name: naplesName, kind: EntityKindNaples},
		withQemu: qemu, passThroughMode: passThroughMode}
	infraCtx.res[naplesName] = &naplesentity
	if qemu {
		qemuName := "Qemu_" + name
		qemuentity := qemuEntity{remoteEntity{name: qemuName, kind: EntityKindQemu}}
		infraCtx.res[qemuName] = &qemuentity
		qemuentity.parent = &vmentity
		vmentity.hasQemu = true
	}
	naplesentity.parent = &vmentity
}

func (infraCtx *infraCtx) initApp(name string, parent string) {
	appentity := appEntity{remoteEntity: remoteEntity{name: name, kind: EntityKindContainer}}
	if parentEntity, ok := infraCtx.res[parent]; ok {
		appentity.parent = parentEntity
		infraCtx.res[name] = &appentity
		appentity.onQemu = parentEntity.(*vmEntity).hasQemu
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

	var warmd public.WarmdEnv
	var err error
	err = json.Unmarshal(file, &warmd)
	if err != nil {
		log.Fatal(err)
		return err
	}

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)

	if len(vmEntities) > len(warmd.Instances) {
		err := fmt.Errorf("User topo has more vm Entities %d than wamrd %d", len(vmEntities),
			len(warmd.Instances))
		infraCtx.logger.Printf(err.Error())
		return err
	}

	cnt := 0
	for _, y := range warmd.Instances {
		vm, ok := vmEntities[cnt].(*vmEntity)
		if !ok {
			return fmt.Errorf("invalid vm entity %+v", vmEntities[cnt])
		}
		vm.ipAddress = y.NodeMgmtIP
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
	return Helpers.RunSSHCommand(vm.SSHHandle, cmd, sudo, bg, vm.logger)
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
	nodeid := hostNode.(*vmEntity).nodeID

	go func() {
		/* TODO : Have to pass on num nodes if more than 2 */
		naplesSimConfg := &NodeService.NaplesSimConfig{
			Name: NaplesSimName, NodeID: uint32(nodeid),
			CtrlNwIPRange:   "11.1.1." + strconv.Itoa(32+32*nodeid) + "/27",
			WithQemu:        naples.withQemu,
			PassThroughMode: naples.passThroughMode,
		}

		if naples.passThroughMode {
			for _, intf := range passedThroughModeIntfs {
				naplesSimConfg.DataIntfs = append(naplesSimConfg.DataIntfs, intf)
			}
		} else {
			for _, intf := range tunneledModeIntfs {
				naplesSimConfg.DataIntfs = append(naplesSimConfg.DataIntfs, intf)
			}
		}

		err := hostNode.(*vmEntity).services.Naples.BringUp(ctx, naplesSimConfg)
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

// bring up method for qemu
func (qemu *qemuEntity) bringUp(ctx context.Context, errChannel chan error) {
	var err error
	var hostNode RemoteEntity

	hostNode, err = qemu.GetHostingNode()
	if err != nil {
		/* PANIC as something weird we did in parsing topo file */
		panic("Unable to find Qemu Hosting node for :" + qemu.Name())
	}

	qemu.log("Doing qemu bring up on Node")

	sshRunError := make(chan error, 0)
	go func() {

		/* First clean up current platform gen */
		rmDir := []string{"rm", "-rf", remotePlatformSrc}
		ret, _, _ := hostNode.(*vmEntity).Exec(strings.Join(rmDir, " "), true, false)
		if ret != 0 {
			log.Printf("failed to remove directory : %s", remotePlatformSrc)
		}

		/* First copy the compiled gen directory to remote */
		if err := Helpers.MultiSFTP(localPlatfromSrc, remoteSrc,
			[]*sftp.Client{hostNode.(*vmEntity).SftpHandle}); err != nil {
			sshRunError <- errors.Wrap(err, "Copying Platfrom Gen failed for  "+hostNode.Name())
		}

		/* TODO : Have to pass on num nodes if more than 2 */
		err := hostNode.(*vmEntity).services.Qemu.BringUp(ctx, &NodeService.QemuConfig{
			Name:  "Qemu image",
			Image: qemuImage,
		})
		if err != nil {
			sshRunError <- errors.Wrap(err, "Qemu bring up failed.")
			return
		}
		time.Sleep(NaplesAgentBringUpDelay)
		sshRunError <- nil
	}()

	select {
	// we received the signal of cancelation in this channel
	case err := <-sshRunError:
		if err != nil {
			log.Print("Qemu bring up failed on : " + hostNode.Name())
			errChannel <- err
		} else {
			time.Sleep(time.Second * 5)
			log.Print("Qemu bring up sucessfull : " + hostNode.Name())
		}
		break
	case <-ctx.Done():
		fmt.Println("Timeout : Canceling Qemu bring up.")
		errChannel <- fmt.Errorf("Qemu bring up timeout on : %s", hostNode.Name())
	}

	errChannel <- nil
}

func (qemu *qemuEntity) teardown(ctx context.Context, errChannel chan error) {
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

	appConfig := &NodeService.AppConfig{Name: appNode.Name(),
		Registry: e2eRegistry, OnQemu: appNode.onQemu}

	err = hostNode.(*vmEntity).services.App.BringUp(ctx, appConfig)

	if err != nil {
		errChannel <- errors.Wrap(err, "Agent bring up api Failed on App")
		return
	}

	appNode.log("Successfull bring up of App :" + appNode.Name())
	errChannel <- err
}

func (appNode *appEntity) configure(ep cfg.Endpoint, config *cfg.E2eCfg, stationDevices []ionic.StationDevice) error {
	getNetworkFromConfig := func(nwName string, fullCfg *cfg.E2eCfg) *cfg.Network {
		for _, network := range fullCfg.NetworksInfo.Networks {
			if network.NetworkMeta.Name == nwName {
				return &network
			}
		}

		return nil
	}

	getStationMac := func(lifID string) string {
		for _, sdevice := range stationDevices {
			sLifID := fmt.Sprintf("lif%d", sdevice.LifID)
			if sLifID == lifID {
				return sdevice.MacAddr
			}
		}
		return ""
	}

	network := getNetworkFromConfig(ep.EndpointSpec.NetworkName, config)
	if network == nil {
		log.Fatalln("Network not found in config!", ep.EndpointSpec.NetworkName)
	}

	hostNode, _ := appNode.GetHostingNode()

	appNode.ipAddress = strings.Split(ep.EndpointSpec.Ipv4Address, "/")[0]
	prefixLen, _ := strconv.Atoi(strings.Split(network.NetworkSpec.Ipv4Subnet, "/")[1])
	parentMac := getStationMac(ep.EndpointSpec.Interface)
	if parentMac == "" {
		return errors.Errorf("Invalid lif ID : %s, not part of station device", ep.EndpointSpec.Interface)
	}
	err := hostNode.(*vmEntity).services.App.AddVlanInterface(context.Background(),
		appNode.Name(),
		&NodeService.AppVlanInterface{
			ParentMacMacAddress: parentMac,
			MacAddress:          ep.EndpointSpec.MacAddresss,
			Vlan:                uint32(ep.EndpointSpec.UsegVlan),
			IPaddress:           appNode.ipAddress,
			PrefixLen:           uint32(prefixLen)})

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

func (vm *vmEntity) bringUp(ctx context.Context, errChannel chan error) {
	vm.log("Bringing up Vm entity : " + vm.Name())
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
		fmt.Println("AGENT SRC " + agentSrc)
		os.Chdir(agentSrc)
		defer os.Chdir(cwd)
		cmd := []string{"go", "build", "-o", RemoteInfraDirectory + "/" + agentExecName, "."}
		env := append(linuxBuildEnv)
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

func (infraCtx *infraCtx) verifyNaplesConnectivity() error {

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)

	if len(vmEntities) < 2 {
		infraCtx.logger.Println("Number of nodes less 2 , skipping connectivity check.")
		return nil
	}
	for _, vm := range vmEntities {
		for i := 0; i < len(vmEntities); i++ {
			nextIP, _ := Helpers.IncrementIP(tunnelIPStart, tunnelIPSubnet, byte(i))
			pingCmd := "ping -c 5 " + nextIP
			stdout, _, err := vm.(*vmEntity).services.Naples.RunCommand(context.Background(), pingCmd)
			infraCtx.logger.Println(stdout)
			if err != nil {
				return errors.Errorf("Ping to destination %s failed from %s", nextIP, vm.Name())
			}
		}
	}
	infraCtx.logger.Println("Data Path connectivity verfied")
	return nil
}

func (infraCtx *infraCtx) resetVMEntities() error {

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)
	for _, vm := range vmEntities {
		vm.(*vmEntity).log("Resetting Vm entity :" + vm.Name())
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

	if err := doBringUp(infraCtx.FindRemoteEntity(EntityKindQemu)); err != nil {
		return err
	}

	if err := doBringUp(infraCtx.FindRemoteEntity(EntityKindContainer)); err != nil {
		return err
	}

	/*Finally verify Vm Connectivity */
	if err := infraCtx.verifyNaplesConnectivity(); err != nil {
		return errors.Wrap(err, "VM connectivity verification failed!")
	}
	return nil
}

func (infraCtx *infraCtx) CleanUp() error {
	ch := make(chan interface{})
	go func() {
		wg := sync.WaitGroup{}
		entities := infraCtx.FindRemoteEntity("")
		errChannel := make(chan error, len(entities))
		for _, entity := range entities {
			wg.Add(1)
			go func(entity RemoteEntity) {
				ctx, cancel := context.WithTimeout(context.Background(), NodeTeardownTimeout)
				defer cancel()
				fmt.Println("Teardown ", entity.Name())
				entity.teardown(ctx, errChannel)
				wg.Done()
			}(entity)
		}
		wg.Wait()

		for i := 0; i < len(entities); i++ {
			if err := <-errChannel; err != nil {
				/* Report and continue */
				log.Printf("Error in teardown %v\n", err)
			}
		}

		infraCtx.resetVMEntities()
		close(ch)
	}()

	select {
	case <-time.After(NodeTeardownTimeout + 5*time.Second):
		log.Printf("Timeout in teardown")
	case <-ch:
	}

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

func (infraCtx *infraCtx) PrintTopology() {

	vmEntities := infraCtx.FindRemoteEntity(EntityKindVM)
	fmt.Println("************************* TOPOLOGY INFORMATION START  *********************************")
	for _, vm := range vmEntities {
		fmt.Println("\tNode Name  :" + vm.Name())
		ipAddr, _ := vm.(*vmEntity).GetIPAddress()
		fmt.Println("\tIP Address :" + ipAddr)
		fmt.Println("\tUserName   :" + vm.(*vmEntity).userName)
		fmt.Println("\tPassword   :" + vm.(*vmEntity).passwd)
		fmt.Println("\tPassword   :" + ipAddr)
		fmt.Println("\tQemu       :" + strconv.FormatBool(vm.(*vmEntity).hasQemu))
		fmt.Println("\tApps       :")
		for _, app := range infraCtx.FindRemoteEntity(EntityKindContainer) {
			hostNode, _ := app.GetHostingNode()
			if hostNode == vm {
				fmt.Println("\t\t Name       :", app.(*appEntity).Name())
				ip, _ := app.(*appEntity).GetIPAddress()
				fmt.Println("\t\t IP address :", ip)
			}
		}

	}
	fmt.Println("************************* TOPOLOGY INFORMATION END ************************************")

}

// NewInfraCtx initializes/returns an instance of the global context
func NewInfraCtx(topoFile string, warmdFile string, passThroughMode bool) (Context, error) {

	ut := loadUserTopo(topoFile)
	ut.validate()

	infraCtx := &infraCtx{}
	infraCtx.res = make(map[string]RemoteEntity)
	infraCtx.setupLogger()
	for _, node := range ut.Nodes {
		switch node.Node.Kind {
		case DataNode:
			infraCtx.initDataNode(node.Node.Name, node.Node.Qemu, passThroughMode)
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
