package agent

type commandCtx struct {
	done      bool
	stdout    string
	handleKey string
	stderr    string
	exitCode  int32
}

type commandHandle struct {
	ctx    *commandCtx
	handle interface{}
}

type entity interface {
	Name() string
	BringUp(args ...string) error
	RunCommand(cmd []string, timeout uint32, background bool, shell bool) (*commandCtx, error)
	StopCommand(commandHandle string) (*commandCtx, error)
	AddInterface(name string, macAddress string, ipaddress string, vlan int) error
	MoveInterface(name string) error
	IsHealthy() bool
	SendArpProbe(ip string, intf string, vlan int) error
	TearDown()
}
