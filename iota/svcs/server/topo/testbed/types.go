package testbed

import (
	"context"
	"os/exec"
	"sync"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/agent/workload"
	common "github.com/pensando/sw/iota/svcs/common"
	constants "github.com/pensando/sw/iota/svcs/common"
	vmware "github.com/pensando/sw/iota/svcs/common/vmware"
	log "github.com/sirupsen/logrus"
)

var (
	logger   *log.Logger
	imageRep imageRepository
)

// TestNode wraps an iota TestNode
type TestNode struct {
	sync.Mutex
	SSHClient *ssh.Client
	Node      *iota.Node
	RespNode  *iota.Node
	//Workloads   []*iota.Workload
	GrpcClient   *common.GRPCClient
	AgentClient  iota.IotaAgentApiClient
	WorkloadInfo *iota.WorkloadMsg
	WorkloadResp *iota.WorkloadMsg
	CimcUserName string
	CimcPassword string
	CimcIP       string
	info         NodeInfo
	//Use when workload are local managed rather than agent
	controlNode  TestNodeInterface
	workloadMap  *sync.Map
	logger       *log.Logger
	connector    interface{}
	triggerLocal bool
	ApcInfo      *iota.ApcInfo
}

// VcenterNode implements functions for Vcenter
type VcenterNode struct {
	TestNode
	managedNodes     map[string]ManagedNodeInterface
	independentNodes map[string]ManagedNodeInterface
	vc               *vmware.Vcenter
	dc               *vmware.DataCenter
	cl               *vmware.Cluster
	imagesMap        map[string]string
	workloads        map[string]workload.Workload
	license          string
}

type imageRepository struct {
	sync.Mutex
	imagesMap map[string]string
}

func (i *imageRepository) GetImageDir(name string) (string, error) {
	i.Lock()
	defer i.Unlock()
	if img, ok := i.imagesMap[name]; ok {
		return img, nil
	}

	return "", errors.New("Image not found")
}

func (i *imageRepository) downloadDataVMImage(image string) (string, error) {

	dataVMDir := constants.IotaServerImgDir + "/" + image
	dstImage := dataVMDir + "/" + image + ".ova"
	mkdir := []string{"mkdir", "-p", dataVMDir}
	if stdout, err := exec.Command(mkdir[0], mkdir[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	buildIt := []string{common.BuildItBinary, "-t", constants.BuildItURL, "image", "pull", "-o", dstImage, image}
	if stdout, err := exec.Command(buildIt[0], buildIt[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	logger.Info("Download complete for VM image")
	tarCmd := []string{"tar", "-xvf", dstImage, "-C", dataVMDir}
	if stdout, err := exec.Command(tarCmd[0], tarCmd[1:]...).CombinedOutput(); err != nil {
		return "", errors.Wrap(err, string(stdout))
	}

	return dataVMDir, nil
}

func (i *imageRepository) DownloadImage(name string) error {

	i.Lock()
	defer i.Unlock()
	if _, ok := i.imagesMap[name]; ok {
		return nil
	}

	dir, err := i.downloadDataVMImage(name)
	if err != nil {
		return err
	}
	i.imagesMap[name] = dir
	return nil
}

// EsxNode implements functions for esx node
type EsxNode struct {
	TestNode
}

//TestNodeInterface interface for all node operations
type TestNodeInterface interface {
	InitNode(reboot bool, c *ssh.ClientConfig, commonArtifacts []string) error
	CleanUpNode(cfg *ssh.ClientConfig, reboot bool) error
	CopyTo(cfg *ssh.ClientConfig, entity string, files []string) error
	CopyFrom(cfg *ssh.ClientConfig, dstDir string, files []string) error
	EntityCopy(cfg *ssh.ClientConfig, req *iota.EntityCopyMsg) (*iota.EntityCopyMsg, error)
	SetupNode() error
	AddNode() error
	GetAgentURL() (string, error)
	GetNodeIP() (string, error)
	IpmiNodeControl(name string, restoreState bool, method string) error
	ReloadNode(name string, restoreState bool, method string) error
	SetNodeMsg(*iota.Node)
	GetNodeMsg(name string) *iota.Node
	GetNodeInfo() NodeInfo
	AddWorkloads(req *iota.WorkloadMsg) (*iota.WorkloadMsg, error)
	DeleteWorkloads(req *iota.WorkloadMsg) (*iota.WorkloadMsg, error)
	GetWorkloads(name string) []*iota.Workload
	NodeController() TestNodeInterface
	SetNodeController(TestNodeInterface)

	TriggerWithContext(ctx context.Context,
		triggerReq *iota.TriggerMsg) (*iota.TriggerMsg, error)
	GetControlledNode(ip string) TestNodeInterface
	GetManagedNodes() []TestNodeInterface
	AssocaiateIndependentNode(node TestNodeInterface) error

	CheckHealth(ctx context.Context, heatth *iota.NodeHealth) (*iota.NodeHealth, error)
	NodeConnector() interface{}
	SetConnector(interface{})

	GetNodeAgent() iota.IotaAgentApiClient
	SetNodeAgent(iota.IotaAgentApiClient)
	RunTriggerLocally()

	IsOrchesratorNode() bool

	MoveWorkloads(ctx context.Context, req *iota.WorkloadMoveMsg) (*iota.WorkloadMoveMsg, error)
}

//ManagedNodeInterface represent interface for a managed node
type ManagedNodeInterface interface {
	TestNodeInterface

	GetHostInterfaces() ([]string, error)
}

//NodeInfo Encapsulates all node information
type NodeInfo struct {
	Name           string
	Os             iota.TestBedNodeOs
	CimcUserName   string
	CimcPassword   string
	CimcIP         string
	ApcInfo        *iota.ApcInfo
	IPAddress      string
	Username       string
	Password       string
	SSHCfg         *ssh.ClientConfig
	ManagedNodes   map[string]NodeInfo
	License        string
	GlobalLicenses []*iota.License
}

//NewTestNode Create a new node interface
func NewTestNode(info NodeInfo) TestNodeInterface {

	switch info.Os {
	case iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD:
		fallthrough
	case iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX:
		return &TestNode{info: info, logger: logger, workloadMap: new(sync.Map)}
	case iota.TestBedNodeOs_TESTBED_NODE_OS_ESX:
		return &EsxNode{TestNode: TestNode{info: info, logger: logger, workloadMap: new(sync.Map)}}
	case iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER:
		return &VcenterNode{TestNode: TestNode{info: info, logger: logger, workloadMap: new(sync.Map)}}
	}
	return nil
}

//IsOrchesratorNode return trun if node is an orchestrator
func (n *TestNode) IsOrchesratorNode() bool {
	return n.info.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER
}

func init() {

	logger = log.New()
	imageRep = imageRepository{imagesMap: make(map[string]string)}

}
