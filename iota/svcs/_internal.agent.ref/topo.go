package agent

import (
	"context"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"

	Helpers "github.com/pensando/sw/iota/common"
	Globals "github.com/pensando/sw/iota/svcs/globals"
)

const (
	NodeBringUpTimeout  = 900 * time.Second
	NodeTeardownTimeout = 120 * time.Second
	userName            = "vm"
	userPassword        = "vm"
)

var (
	//LogDir for infra
	LogDir = "/tmp/iota-server/"

	//SrcVeniceDirectory Venice image source directory
	SrcVeniceDirectory = "/sw/bin"
	//SrcNaplesDirectory Naples image source directory
	SrcNaplesDirectory = "/sw/nic/obj/images"
	localPlatfromSrc   = "/sw/platform"

	remoteSrc         = "/home/vm/sw"
	remotePlatformSrc = remoteSrc + "/platform"
)

// Entity is an abstraction that represents a node that can be naples, venice, application container/node
type Entity interface {
	Name() string

	Kind() string

	Exec(cmd string, sudo, background bool) (retCode int, stdout, stderr []string)

	Init() error

	Setup(ctx context.Context, err chan error)

	Teardown(ctx context.Context, err chan error)

	Bringup(ctx context.Context, err chan error)
}

//TopoContext - Context to hold topology information
type TopoContext interface {
	FindEntity(kind string) []Entity

	FindEntityByName(name string) Entity

	InitTestBed(entities []Entity) error

	CleanUpTestBed(entities []Entity) error

	AddNodes(entities []Entity, makeCluster bool) error

	DeleteNodes(entities []Entity) error

	GetClusterHealth() error

	/* Print Topology of infra */
	PrintTopology()
}

// entity defines an abstracted remote entity such as naples, venice, host, app container, etc.
type entity struct {
	name      string      // name of the node, used for remote execution
	kind      string      // could be venice, naples, workload, or switch
	ipAddress string      // ip address of the node, or parent node
	parent    Entity      // parent remote entity e.g. host on which node/container is running
	logger    *log.Logger // Logger for each entity.
}

// ctx is instantiation of TopoContext interface, used by test cases to fetch infra details
type topoCtx struct {
	res    map[string]Entity
	mutex  sync.Mutex
	logger *log.Logger
}

// FindEntity - finds a remote entity of a specified kind, if none specified it returns all remote entities
func (ctx *topoCtx) FindEntity(kind string) []Entity {
	newRes := []Entity{}

	if kind == "" {
		all := make([]Entity, 0, len(ctx.res))

		for _, value := range ctx.res {
			all = append(all, value)
		}
		return all
	}

	for _, entity := range ctx.res {
		if entity.Kind() == kind {
			newRes = append(newRes, entity)
		}
	}

	return newRes
}

// FindEntity - finds a remote entity of a specified kind, if none specified it returns all remote entities
func (ctx *topoCtx) FindEntityByName(name string) Entity {

	for _, entity := range ctx.res {
		if entity.Name() == name {
			return entity
		}
	}

	return nil
}

// Name returns the name of the remote entity
func (e *entity) Name() string {
	return e.name
}

// Kind returns the kind of the remote entity
func (e *entity) Kind() string {
	return e.kind
}

// Exec - executes a command on a remote entity, returns stdout/stderr
// cmd - command string
// stdout, stderr - return stdout/stderr values, limited to 10k lines
func (e *entity) Exec(cmd string, sudo, background bool) (retCode int, stdout, stderr []string) {
	retCode = 0
	stdout = []string{}
	stderr = []string{}

	return
}

func (e *entity) Bringup(ctx context.Context, err chan error) {
	err <- nil
}

func (e *entity) Teardown(ctx context.Context, err chan error) {
	err <- nil
}

//FindEntity fron list find entity of a particular type
func FindEntity(kind string, entities []Entity) []Entity {
	newRes := []Entity{}

	if kind == "" {

		for _, value := range entities {
			newRes = append(newRes, value)
		}
		return newRes
	}

	for _, entity := range entities {
		if entity.Kind() == kind {
			newRes = append(newRes, entity)
		}
	}

	return newRes
}

//nodeEntity node enttity
type nodeEntity struct {
	entity
	SftpHandle *sftp.Client
	SSHHandle  *ssh.Client
}

func (node *nodeEntity) connect(username string, password string) error {
	var err error
	node.SftpHandle, err = Helpers.NewSFTPClient(node.ipAddress,
		username, password)
	if err != nil {
		err = errors.Wrapf(err, "SFTP connect failed")
		node.logger.Println(err.Error())
		return err
	}
	config := &ssh.ClientConfig{
		User: username,
		Auth: []ssh.AuthMethod{
			ssh.Password(password),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	if conn, err := ssh.Dial("tcp", node.ipAddress+":22", config); err == nil {
		node.SSHHandle = conn
	} else {
		err = errors.Wrapf(err, "SSH connect failed")
		node.logger.Println(err.Error())
		return err
	}
	return nil
}

func (node *nodeEntity) Init() error {
	return node.connect(userName, userPassword)
}

func (node *nodeEntity) Exec(cmd string, sudo bool, bg bool) (retCode int, stdout, stderr []string) {
	return Helpers.RunSSHCommand(node.SSHHandle, cmd, sudo, bg, node.logger)
}

func (node *nodeEntity) Setup(ctx context.Context, errChannel chan error) {

	setupAgent := func() error {
		cmdStart := "mkdir -p " + Globals.RunnerDirectory + " && chmod 777  " + Globals.RunnerDirectory
		ret, _, _ := node.Exec(cmdStart, true, false)
		if ret != 0 {
			return errors.Errorf("Failed to create run directory on %s", node.Name())
		}
		err := Helpers.MultiSFTP(iotaAgentBinDir, Globals.RunnerDirectory,
			[]*sftp.Client{node.SftpHandle})
		if err != nil {
			return errors.Wrap(err, "Copying agent binary failed on "+node.Name())
		}
		cmdStart = Globals.RemoteIotaDirectory + "/" + Globals.AgentExecName + " -port=" + strconv.Itoa(Globals.IotaAgentPort)
		fmt.Println("Executing cmd ", cmdStart)
		ret, _, _ = node.Exec(cmdStart, true, true)
		if ret != 0 {
			return errors.Errorf("Agent bring up failed on node : %s", node.Name())
		}
		time.Sleep(1 * time.Second)
		fmt.Println("Connecting to Agent..")

		/*
			if node.services, err = IotaAgent.InitNodeService(node.ipAddress, Globals.IotaAgentPort); err != nil {
				return errors.Wrap(err, "Error in Initializing node services : "+node.ipAddress+":"+strconv.Itoa(Globals.IotaAgentPort))

			} */
		log.Println("Agent bring up successful on node : " + node.ipAddress + ":" + strconv.Itoa(Globals.IotaAgentPort))
		return nil
	}

	copyImages := func() error {

		//Copy Venice Images
		err := Helpers.MultiSFTP(SrcVeniceDirectory, Globals.RemoteVeniceDirectory,
			[]*sftp.Client{node.SftpHandle})
		if err != nil {
			err = errors.Wrapf(err, "Error in copying Venice files to VM : %s", node.Name())
			node.logger.Println(err.Error())
			return err
		}

		//Copy VNaples Images
		err = Helpers.MultiSFTP(SrcNaplesDirectory, Globals.RemoteNaplesDirectory,
			[]*sftp.Client{node.SftpHandle})
		if err != nil {
			err = errors.Wrapf(err, "Error in copying Naples files to VM : %s", node.Name())
			node.logger.Println(err.Error())
			return err
		}

		return nil
	}

	node.logger.Println("Setting up node entity :" + node.Name())

	if err := setupAgent(); err != nil {
		errChannel <- err
		return
	}

	node.logger.Println("Copying images and build artifcats : " + node.Name())
	if err := copyImages(); err != nil {
		errChannel <- err
		return
	}

	node.logger.Println("Setup done : " + node.Name())
	errChannel <- nil
}

func (node *nodeEntity) Teardown(ctx context.Context, errChannel chan error) {

	killAgent := func() error {
		cmd := "pkill -9 " + Globals.AgentExecName
		ret, _, _ := node.Exec(cmd, true, false)
		if ret != 0 {
			node.logger.Println("Killing agent successful on node : " + node.ipAddress + ":" + strconv.Itoa(Globals.IotaAgentPort))
		} else {
			node.logger.Println("Killing agent failed on node : " + node.ipAddress + ":" + strconv.Itoa(Globals.IotaAgentPort))
		}
		return nil
	}

	cleanUpOldState := func() error {
		cmd := "rm -rf " + Globals.RunnerDirectory
		fmt.Println("Running cmd..")
		node.Exec(cmd, true, false)

		cmdStart := "mkdir -p " + Globals.RunnerDirectory + " && chmod 777  " + Globals.RunnerDirectory
		ret, _, _ := node.Exec(cmdStart, true, false)
		if ret != 0 {
			return errors.Errorf("Failed to create run directory on %s", node.Name())
		}
		err := Helpers.MultiSFTP(testScriptsDir, Globals.RunnerDirectory,
			[]*sftp.Client{node.SftpHandle})
		if err != nil {
			return errors.Wrap(err, "Copying Scripts Failed "+node.Name())
		}
		fmt.Println("Running Venice clean up.")
		cmd = Globals.RunnerDirectory + "/scripts/" + "INSTALL.sh --clean-only"
		node.Exec(cmd, true, false)

		/* TODO : Some of this stuff must move to Agent
		 * It might make sense to have agent do.
		 * But this should be a general service, where
		 * cluster
		 */
		cleanUpOld := "ifconfig eth0:pen0 down"
		node.Exec(cleanUpOld, true, false)
		cleanUpOld = "sed -i '/venice-/d' /etc/hosts"
		node.Exec(cleanUpOld, true, false)

		setHostname := "hostnamectl set-hostname localhost"
		node.Exec(setHostname, true, false)

		// have to pass control interface some how
		//resetControlIntf := "ifconfig " + controlIntf + " 0.0.0.0"
		//node.Exec(resetControlIntf, true, false)
		return nil
	}

	stopallContainers := func() error {
		// Stop all running containers
		cmd := "docker ps -a -q"
		ret, stdout, _ := node.Exec(cmd, true, false)
		if ret != 0 {
			node.logger.Println("Failed stop all containers : " + node.ipAddress)
		} else {
			for _, pid := range stdout {
				cmd := "docker stop " + strings.TrimSpace(pid)
				node.Exec(cmd, true, false)
			}
			node.logger.Println("Stopped all containers : " + node.ipAddress)
		}

		return nil
	}

	node.logger.Println("Resetting node entity :" + node.Name())

	if err := killAgent(); err != nil {
		errChannel <- err
		return
	}

	if err := cleanUpOldState(); err != nil {
		errChannel <- nil
		return
	}

	if err := stopallContainers(); err != nil {
		errChannel <- nil
		return
	}

	errChannel <- nil
}

func (*topoCtx) ConnectToEntities(entities []Entity) error {
	for _, entity := range entities {
		if err := entity.Init(); err != nil {
			return err
		}
	}
	return nil
}

//InitTestBed Initalize tesbed
func (ts *topoCtx) InitTestBed(entities []Entity) error {

	if err := ts.ConnectToEntities(entities); err != nil {
		ts.logger.Println("Error in initialzing node entities")
		return err
	}

	errChannel := make(chan error, len(entities))
	for _, entity := range entities {
		ctx, cancel := context.WithTimeout(context.Background(), NodeTeardownTimeout)
		defer cancel()
		fmt.Println("Setting up node ", entity.Name())
		go entity.Setup(ctx, errChannel)
	}

	for i := 0; i < len(entities); i++ {
		if err := <-errChannel; err != nil {
			/* Report and continue */
			log.Printf("Error in teardown %v\n", err)
		}
	}

	return nil
}

//CleanUpTestBed cleanup testbed
func (ts *topoCtx) CleanUpTestBed(entities []Entity) error {
	if err := ts.ConnectToEntities(entities); err != nil {
		ts.logger.Println("Error in initialzing node entities")
		return err
	}
	errChannel := make(chan error, len(entities))
	for _, entity := range entities {
		ctx, cancel := context.WithTimeout(context.Background(), NodeTeardownTimeout)
		defer cancel()
		fmt.Println("Teardown ", entity.Name())
		go entity.Teardown(ctx, errChannel)
	}

	for i := 0; i < len(entities); i++ {
		if err := <-errChannel; err != nil {
			/* Report and continue */
			log.Printf("Error in teardown %v\n", err)
		}
	}

	return nil
}

func (*topoCtx) AddNodes(entities []Entity, makeCluster bool) error {
	return nil

}

func (*topoCtx) DeleteNodes(entities []Entity) error {
	return nil

}

func (*topoCtx) GetClusterHealth() error {
	return nil

}

/* Print Topology of infra */
func (*topoCtx) PrintTopology() {
}

var testScriptsDir string
var iotaAgentBinDir string

func init() {
	// Harcoding to GRPC agent type for now.
	iotaAgentBinDir = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/iota-server/bin/iota-server-agent"
	testScriptsDir = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/tools/scripts"
}

func NewNodeEntity(name string, ipaddress string, logger *log.Logger) Entity {
	return &nodeEntity{entity: entity{name: name, ipAddress: ipaddress, logger: logger}}
}

func NewTopoContext(logger *log.Logger) TopoContext {
	return &topoCtx{logger: logger}
}
