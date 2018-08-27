package infra

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"sync"
	"time"

	cfg "github.com/pensando/sw/test/utils/infra/config"
)

// Remote Entity Kind (or type)
const (
	DataNode      = "datanode"
	ControlNode   = "controlnode"
	ContainerType = "container"
	NaplesSimName = "naples-sim"

	NodeBringUpTimeout  = 300 * time.Second
	NodeTeardownTimeout = 120 * time.Second

	RunnerDirectory       = "/tmp/turin/"
	RemoteInfraDirectory  = RunnerDirectory + "/infra"
	RemoteNaplesDirectory = RunnerDirectory + "/naples"
	NaplesContainerImage  = "naples-release-v1.tgz"
	NaplesVMBringUpScript = "naples_vm_bringup.py"

	agentExecName   = "testagent"
	agentPort       = 9876
	naplesAgentPort = 9007
	InfraLogFile    = "infra.log"

	NaplesAgentBringUpDelay = (20 * time.Second)
)

var (
	//SrcNaplesDirectory Naples image source directory
	SrcNaplesDirectory = "/sw/nic/obj/images"
)

// Context - created by one binary usually or possibly one thread of a test binary
type Context interface {
	FindRemoteEntity(kind string) []RemoteEntity

	GetAppCatalog(match string) []string

	DoConfiguration()

	FindRemoteEntityByName(name string) RemoteEntity
}

// ctx is instantiation of Context interface, used by test cases to fetch infra details
type infraCtx struct {
	res    map[string]RemoteEntity
	mutex  sync.Mutex
	logger *log.Logger
}

// Config specifies the configuration parameters when initializing the package
type Config struct {
}

// New initializes/returns an instance of the global context
func New(cfg Config) (Context, error) {
	return &infraCtx{}, nil
}

// FindRemoteEntity - finds a remote entity of a specified kind, if none specified it returns all remote entities
func (ctx *infraCtx) FindRemoteEntity(kind string) []RemoteEntity {
	newRes := []RemoteEntity{}

	if kind == "" {
		all := make([]RemoteEntity, 0, len(ctx.res))

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

// FindRemoteEntity - finds a remote entity of a specified kind, if none specified it returns all remote entities
func (ctx *infraCtx) FindRemoteEntityByName(name string) RemoteEntity {

	for _, entity := range ctx.res {
		if entity.Name() == name {
			return entity
		}
	}

	return nil
}

// GetAppCatalog - gets all pre-defined apps from the catalog, if match is non null, it returns only apps that contains provided match value
func (ctx *infraCtx) GetAppCatalog(match string) []string {
	return []string{}
}

//DoConfiguration Temp function to push static config for now.
func (ctx *infraCtx) DoConfiguration() {

	naplesEntities := ctx.FindRemoteEntity(EntityKindNaples)

	for id, naples := range naplesEntities {
		configFile := naplesAgentCfgPath + "/node" + strconv.Itoa(id+1) + ".cfg"
		hostNode, _ := naples.GetHostingNode()
		cfg.PushConfiguration(hostNode.(*vmEntity).ipAddress,
			naplesAgentPort, configFile)
		hostApps := ctx.getHostApps(hostNode)
		nodeCfg := cfg.GetAgentConfig(configFile)
		if nodeCfg == nil {
			panic("Erron in getting agent configuration from " + configFile)
		}
		appIndex := 0
		for _, endpoint := range nodeCfg.EndpointsInfo.Endpoints {
			if endpoint.EndpointSpec.InterfaceType == "lif" {
				err := hostApps[appIndex].(*appEntity).configure(endpoint, nodeCfg)
				if err != nil {
					fmt.Println("Configuration failed", err.Error())
				}
				appIndex++
			}
		}

	}

}

func init() {
	agentSrc = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/utils/infra/agent"
	naplesAgentCfgPath = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/utils/infra/test_cfg"
}
