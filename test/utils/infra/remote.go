package infra

import (
	"context"
	"fmt"
	"io"
	"log"
	"os"
	"regexp"
	"time"
)

// RemoteEntity is an abstraction that represents a node that can be naples, venice, application container/vm, switch or a third party tool
type RemoteEntity interface {
	Name() string

	Dump() string

	Kind() string

	Parent() RemoteEntity

	GetConnectivity() []Link

	SpawnApp(appName string) (RemoteEntity, error)

	RegisterRemoteEntityCallback(cb ReCallbackFn, event CallbackEventType) error

	Push(cbuf ConfigBuffer) error

	ScpTo(src, dest string) error

	ScpFrom(src, dest string) error

	GetIPAddress() (string, error)

	Reset() error

	Exec(cmd string, sudo, background bool) (retCode int, stdout, stderr []string)

	GetHostingNode() (RemoteEntity, error)

	Expect(re *regexp.Regexp, timeout time.Duration) (out string, match []string, err error)

	Send(done chan interface{})

	bringUp(ctx context.Context, err chan error)

	teardown(ctx context.Context, err chan error)
}

// Remote Entity Kind (or type)
const (
	EntityKindNaples    = "naples"
	EnityKindVenice     = "venice"
	EntityKindContainer = "container"
	EntityKindVM        = "vm"
	EntityKindSwitch    = "switch"

	CallbackEventRemoteEntityUnresponsive = 1
	CallbackEventTestFailure              = 2
)

var (
	//LogDir for infra
	LogDir = "/var/log/turin/"
)

// Link describes one record that describes the topology
// e.g. Naples can connect to switches, VFs can connect to Workloads
type Link struct {
	PortName string       // VF or Switch port name
	ToEntity RemoteEntity // Remote Entity to which above mentioned port is connected to e.g. Workload/Switch
}

// ReCallbackFn is a function to be called if registered for an event
type ReCallbackFn func(eventType CallbackEventType, re RemoteEntity)

// CallbackEventType is the type of callback events (from defined constants)
type CallbackEventType int

// remoteEntity defines an abstracted remote entity such as naples, venice, host, app container, etc.
type remoteEntity struct {
	name      string       // name of the node, used for remote execution
	kind      string       // could be venice, naples, workload, or switch
	ipAddress string       // ip address of the node, or parent node
	parent    RemoteEntity // parent remote entity e.g. host on which VM/container is running
	logger    *log.Logger  // Logger for each entity.
}

// Name returns the name of the remote entity
func (re *remoteEntity) Name() string {
	return re.name
}

// Kind returns the kind of the remote entity
func (re *remoteEntity) Kind() string {
	return re.kind
}

// Parent returns the parent of the remote entity
func (re *remoteEntity) Parent() RemoteEntity {
	return re.parent
}

// Dump returns debug information about a remote entity
func (re *remoteEntity) Dump() string {
	return fmt.Sprintf("Node: %s, Kind: %s, IP: %s", re.name, re.kind, re.ipAddress)
}

// GetConnectivity - gets connected remote entities to a given remote entity
func (re *remoteEntity) GetConnectivity() []Link {
	return []Link{}
}

// Spawn - spawns an application on the specified remote entity
// appName - application name predefined catalog of applications
// RemoteEntity (return) - handle to the app to perform operations on it later
func (re *remoteEntity) SpawnApp(appName string) (RemoteEntity, error) {
	newRe := &remoteEntity{}

	return newRe, nil
}

// RegisterRemoteEntityCallback specifies interest in getting to know a specific event by the test
// TBD: should we ask for a channel instead?
func (re *remoteEntity) RegisterRemoteEntityCallback(cb ReCallbackFn, event CallbackEventType) error {
	return nil
}

// Push sends the configuration in the buffer to a specific remote entity
// or to all nodes
func (re *remoteEntity) Push(cbuf ConfigBuffer) error {
	return nil
}

// ScpTo - copies files to a remote entity from a local drive
func (re *remoteEntity) ScpTo(src, dest string) error {
	return nil
}

// ScpFrom - copies files from a remote entity to local drive
func (re *remoteEntity) ScpFrom(src, dest string) error {
	return nil
}

// Reset - power-cycle a bm, power down a vm, kill a container, etc.
// typically used when remote entity is perceived unresponsive or to simulate a hard failure
func (re *remoteEntity) Reset() error {
	return nil
}

// Exec - executes a command on a remote entity, returns stdout/stderr
// cmd - command string
// stdout, stderr - return stdout/stderr values, limited to 10k lines
func (re *remoteEntity) Exec(cmd string, sudo, background bool) (retCode int, stdout, stderr []string) {
	retCode = 0
	stdout = []string{}
	stderr = []string{}

	return
}

// GetHostingNode - returns reference to hosting node (e.g. VM/host of a container), if applicable
func (re *remoteEntity) GetHostingNode() (RemoteEntity, error) {
	if re.Parent() != nil {
		return re.Parent(), nil
	}
	return nil, fmt.Errorf("No Hosting node")
}

// Expect - waits to match a given regex output for a command
func (re *remoteEntity) Expect(regex *regexp.Regexp, timeout time.Duration) (out string, match []string, err error) {
	return "", []string{}, nil
}

// Send - sends a command over the pty to the remote entity
func (re *remoteEntity) Send(done chan interface{}) {
}

func (re *remoteEntity) bringUp(ctx context.Context, err chan error) {

}

func (re *remoteEntity) GetIPAddress() (string, error) {
	return "", nil
}

func (re *remoteEntity) teardown(ctx context.Context, err chan error) {

}

func (re *remoteEntity) log(msg interface{}) {

	if re.logger == nil {
		file, err := os.OpenFile(LogDir+"/"+re.Name()+".log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
		if err != nil {
			log.Fatalln("Failed to open log file", "file.txt", ":", err)
		}
		multi := io.MultiWriter(file, os.Stdout)
		re.logger = log.New(multi, re.Name()+"-"+re.ipAddress+":", log.Ldate|log.Ltime)
	}
	re.logger.Println(msg)
}
