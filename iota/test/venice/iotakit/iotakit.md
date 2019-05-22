# iotakit
--
    import "github.com/pensando/sw/iota/test/venice/iotakit"

Package iotakit is a wrapper library for writing iota tests. This library
provides an abstract model of the system under test. Tests can configure venice
or perform actions on the cluster by manipulating this abstract model called
"SysModel".

Most of the logic of changing configuration, verifying status and performing
triggers is done by the SysModel. SysModel allows us to write test cases using
high level primitives instead of object CRUD primitives. SysModel also has a
very expressive query language to iterate over the model and perform operations
on them.

## Contents

1. [SysMode](#SysModel)
2. [Actions](#Actions)
3. [Collections](#Collections)
4. [Testbed API](#testbed-api)

## SysModel

#### type SysModel

    type SysModel struct {
    }


SysModel represents a model of the system under test

#### func  NewSysModel

    func NewSysModel(tb *TestBed) (*SysModel, error)

NewSysModel creates a sysmodel for a testbed

#### func (*SysModel) Action

    func (sm *SysModel) Action() *ActionCtx

Action returns an interface of actions

#### func (*SysModel) ForEachHost

    func (sm *SysModel) ForEachHost(fn hostIteratorFn) error

ForEachHost calls a function for each host

#### func (*SysModel) ForEachNaples

    func (sm *SysModel) ForEachNaples(fn naplesIteratorFn) error

ForEachNaples calls an iterator for each naples in the model

#### func (*SysModel) ForEachVeniceNode

    func (sm *SysModel) ForEachVeniceNode(fn veniceNodeIteratorFn) error

ForEachVeniceNode runs an iterator function on each venice node

#### func (*SysModel) Hosts

    func (sm *SysModel) Hosts() *HostCollection

Hosts returns list of all hosts in the system

#### func (*SysModel) Networks

    func (sm *SysModel) Networks() *NetworkCollection

Networks returns a list of subnets

#### func (*SysModel) NewSGPolicy

    func (sm *SysModel) NewSGPolicy(name string) *SGPolicyCollection

NewSGPolicy creates a new SG policy

#### func (*SysModel) SGPolicy

    func (sm *SysModel) SGPolicy(name string) *SGPolicyCollection

SGPolicy finds an SG policy by name

#### func (*SysModel) SetupDefaultConfig

    func (sm *SysModel) SetupDefaultConfig() error

SetupDefaultConfig sets up a default config for the system

#### func (*SysModel) VeniceNodes

    func (sm *SysModel) VeniceNodes() *VeniceNodeCollection

VeniceNodes returns a collection of venice nodes

#### func (*SysModel) WorkloadPairs

    func (sm *SysModel) WorkloadPairs() *WorkloadPairCollection

WorkloadPairs creates full mesh of workload pairs

#### func (*SysModel) Workloads

    func (sm *SysModel) Workloads() *WorkloadCollection

Workloads returns all workloads in the model


#### func (*SysModel) AddNaplesNode

    func (sm *SysModel) AddNaplesNode(name string) *error

AddNaples will add naples node and changes to network managed mode.

#### func (*SysModel) DeleteNaplesNode

    func (sm *SysModel) DeleteNaplesNode(name string) *error

DeleteNaplesNode will delete naples node and changes to self managed mode.

## Actions

#### type ActionCtx

    type ActionCtx struct {
    }


ActionCtx is the internal state of the actions

#### func (*ActionCtx) ConnectNaples

    func (act *ActionCtx) ConnectNaples(npc *NaplesCollection) error

ConnectNaples connects naples back to venice by bringing up control interface

#### func (*ActionCtx) DisconnectNaples

    func (act *ActionCtx) DisconnectNaples(npc *NaplesCollection) error

DisconnectNaples disconnects naples by bringing down its control interface

#### func (*ActionCtx) PingFails

    func (act *ActionCtx) PingFails(wpc *WorkloadPairCollection) error

PingFails action verifies ping fails between collection of workload pairs

#### func (*ActionCtx) PingPairs

    func (act *ActionCtx) PingPairs(wpc *WorkloadPairCollection) error

PingPairs action verifies ping works between collection of workload pairs

#### func (*ActionCtx) ReloadHosts

    func (act *ActionCtx) ReloadHosts(hc *HostCollection) error

ReloadHosts reloads a host

#### func (*ActionCtx) ReloadNaples

    func (act *ActionCtx) ReloadNaples(npc *NaplesCollection) error

ReloadNaples reloads naples nodes

#### func (*ActionCtx) ReloadVeniceNodes

    func (act *ActionCtx) ReloadVeniceNodes(vnc *VeniceNodeCollection) error

ReloadVeniceNodes reloads a venice node

#### func (*ActionCtx) TCPSession

    func (act *ActionCtx) TCPSession(wpc *WorkloadPairCollection, port int) error

TCPSession runs TCP session between pair of workloads

#### func (*ActionCtx) TCPSessionFails

    func (act *ActionCtx) TCPSessionFails(wpc *WorkloadPairCollection, port int) error

TCPSessionFails verifies TCP session fails between pair of workloads

#### func (*ActionCtx) UDPSession

    func (act *ActionCtx) UDPSession(wpc *WorkloadPairCollection, port int) error

UDPSession runs UDP session between pair of workloads

#### func (*ActionCtx) UDPSessionFails

    func (act *ActionCtx) UDPSessionFails(wpc *WorkloadPairCollection, port int) error

UDPSessionFails verifies UDP session fails between pair of workloads

#### func (*ActionCtx) VerifyClusterStatus

    func (act *ActionCtx) VerifyClusterStatus() error

VerifyClusterStatus verifies venice cluster status

#### func (*ActionCtx) VerifyPolicyStatus

    func (act *ActionCtx) VerifyPolicyStatus(spc *SGPolicyCollection) error

VerifyPolicyStatus verifies SG policy status

#### func (*ActionCtx) VerifyWorkloadStatus

    func (act *ActionCtx) VerifyWorkloadStatus(wc *WorkloadCollection) error

VerifyWorkloadStatus verifies workload status in venice


## Collections

#### type HostCollection

    type HostCollection struct {
    }


HostCollection is collection of hosts

#### func (*HostCollection) NewWorkload

    func (hc *HostCollection) NewWorkload(namePrefix string, snc *NetworkCollection) *WorkloadCollection

NewWorkload creates new workload on the host in each specified subnet


#### type NaplesCollection

    type NaplesCollection struct {
    }


NaplesCollection contains a list of naples nodes

#### type Network

    type Network struct {
    	Name string // subnet name
    }


Network represents a Vlan with a subnet (called network in venice)

#### type NetworkCollection

    type NetworkCollection struct {
    }


NetworkCollection is a list of subnets

#### func (*NetworkCollection) Any

    func (snc *NetworkCollection) Any(num int) *NetworkCollection

Any returns any one of the subnets in random


#### type SGPolicyCollection

    type SGPolicyCollection struct {
    }


SGPolicyCollection is list of policies

#### func (*SGPolicyCollection) AddRule

    func (spc *SGPolicyCollection) AddRule(fromIP, toIP, port, action string) *SGPolicyCollection

AddRule adds a rule to the policy

#### func (*SGPolicyCollection) AddRulesForWorkloadPairs

    func (spc *SGPolicyCollection) AddRulesForWorkloadPairs(wpc *WorkloadPairCollection, port, action string) *SGPolicyCollection

AddRulesForWorkloadPairs adds rule for each workload pair into the policies

#### func (*SGPolicyCollection) Commit

    func (spc *SGPolicyCollection) Commit() error

Commit writes the policy to venice

#### func (*SGPolicyCollection) Delete

    func (spc *SGPolicyCollection) Delete() error

Delete deletes all policies in the collection

#### func (*SGPolicyCollection) Rules

    func (spc *SGPolicyCollection) Rules() *SGRuleCollection

Rules returns a list of all rule in policy collection

#### type SGRuleCollection

    type SGRuleCollection struct {
    }


SGRuleCollection is list of rules

#### func (*SGRuleCollection) Commit

    func (rc *SGRuleCollection) Commit() error

Commit writes all policies associates with these rules to venice

#### func (*SGRuleCollection) Update

    func (rc *SGRuleCollection) Update(updateCmd string) *SGRuleCollection

Update will update a list of rules. It expects update in the format `field =
value`



#### type VeniceNodeCollection

    type VeniceNodeCollection struct {
    }


VeniceNodeCollection is collection of venice nodes

#### func (*VeniceNodeCollection) Leader

    func (vnc *VeniceNodeCollection) Leader() *VeniceNodeCollection

Leader returns the leader node


#### type WorkloadCollection

    type WorkloadCollection struct {
    }


WorkloadCollection is the collection of workloads

#### func (*WorkloadCollection) Bringup

    func (wc *WorkloadCollection) Bringup() error

Bringup brings up all workloads in the collection

#### func (*WorkloadCollection) Delete

    func (wc *WorkloadCollection) Delete() error

Delete deletes each workload in the collection

#### func (*WorkloadCollection) Error

    func (wc *WorkloadCollection) Error() error

Error returns the error in collection

#### func (*WorkloadCollection) HasError

    func (wc *WorkloadCollection) HasError() bool

HasError returns true if the collection has error

#### func (*WorkloadCollection) MeshPairs

    func (wc *WorkloadCollection) MeshPairs() *WorkloadPairCollection

MeshPairs returns full-mesh pair of workloads



WorkloadPair is a pair of workloads

#### type WorkloadPairCollection

    type WorkloadPairCollection struct {
    }


WorkloadPairCollection is collection of workload pairs

#### func (*WorkloadPairCollection) Any

    func (wpc *WorkloadPairCollection) Any(num int) *WorkloadPairCollection

Any returna one pair from the collection in random

#### func (*WorkloadPairCollection) Error

    func (wpc *WorkloadPairCollection) Error() error

Error returns the collection error

#### func (*WorkloadPairCollection) HasError

    func (wpc *WorkloadPairCollection) HasError() bool

HasError returns true if collection has an error

#### func (*WorkloadPairCollection) ReversePairs

    func (wpc *WorkloadPairCollection) ReversePairs() *WorkloadPairCollection

ReversePairs reverses the pairs by swapping first and second entries

#### func (*WorkloadPairCollection) WithinNetwork

    func (wpc *WorkloadPairCollection) WithinNetwork() *WorkloadPairCollection

WithinNetwork filters workload pairs to only withon subnet


## Trigger

#### type Trigger

    type Trigger struct {
    }


Trigger is an instance of trigger

#### func (*Trigger) AddBackgroundCommand

    func (tr *Trigger) AddBackgroundCommand(command, entity, node string) error

AddBackgroundCommand adds a background command

#### func (*Trigger) AddCommand

    func (tr *Trigger) AddCommand(command, entity, node string) error

AddCommand adds a command to trigger

#### func (*Trigger) Run

    func (tr *Trigger) Run() ([]*iota.Command, error)

Run runs trigger commands in parallel

#### func (*Trigger) RunSerial

    func (tr *Trigger) RunSerial() ([]*iota.Command, error)

RunSerial runs commands serially

#### func (*Trigger) StopCommands

    func (tr *Trigger) StopCommands(cmds []*iota.Command) ([]*iota.Command, error)

StopCommands stop all commands using previously returned command handle




## Testbed API

#### type TestBed

    type TestBed struct {
    	Topo   Topology      // testbed topology
    	Params TestBedParams // testbed params - provided by warmd
    	Nodes  []*TestNode   // nodes in the testbed
    }


TestBed is the state of the testbed

#### func  NewTestBed

    func NewTestBed(topoName string, paramsFile string) (*TestBed, error)

NewTestBed initializes a new testbed and returns a testbed handler

#### func (*TestBed) CheckIotaClusterHealth

    func (tb *TestBed) CheckIotaClusterHealth() error

CheckIotaClusterHealth checks iota cluster health

#### func (*TestBed) CheckNaplesHealth

    func (tb *TestBed) CheckNaplesHealth(node *Naples) error

CheckNaplesHealth checks if naples is healthy

#### func (*TestBed) CheckVeniceServiceStatus

    func (tb *TestBed) CheckVeniceServiceStatus(leaderNode string) error

CheckVeniceServiceStatus checks if all services are running on venice nodes

#### func (*TestBed) Cleanup

    func (tb *TestBed) Cleanup() error

Cleanup cleans up the testbed

#### func (*TestBed) CollectLogs

    func (tb *TestBed) CollectLogs() error

CollectLogs collects all logs files from the testbed

#### func (*TestBed) CopyFromHost

    func (tb *TestBed) CopyFromHost(nodeName string, files []string, destDir string) error

CopyFromHost copies a file from host

#### func (*TestBed) CopyFromNaples

    func (tb *TestBed) CopyFromNaples(nodeName string, files []string, destDir string) error

CopyFromNaples copies files from naples

#### func (*TestBed) CopyFromVenice

    func (tb *TestBed) CopyFromVenice(nodeName string, files []string, destDir string) error

CopyFromVenice copies a file from venice node

#### func (*TestBed) CopyToHost

    func (tb *TestBed) CopyToHost(nodeName string, files []string, destDir string) error

CopyToHost copies a file to host

#### func (*TestBed) CreateFirewallProfile

    func (tb *TestBed) CreateFirewallProfile(fwp *security.FirewallProfile) error

CreateFirewallProfile creates firewall profile

#### func (*TestBed) CreateHost

    func (tb *TestBed) CreateHost(host *cluster.Host) error

CreateHost creates host object in venice

#### func (*TestBed) CreateSGPolicy

    func (tb *TestBed) CreateSGPolicy(sgp *security.SGPolicy) error

CreateSGPolicy creates SG policy

#### func (*TestBed) CreateWorkload

    func (tb *TestBed) CreateWorkload(wrkld *workload.Workload) error

CreateWorkload creates workload

#### func (*TestBed) DeleteSGPolicy

    func (tb *TestBed) DeleteSGPolicy(sgp *security.SGPolicy) error

DeleteSGPolicy deletes SG policy

#### func (*TestBed) DeleteWorkload

    func (tb *TestBed) DeleteWorkload(wrkld *workload.Workload) error

DeleteWorkload deletes workload

#### func (*TestBed) GetCluster

    func (tb *TestBed) GetCluster() (cl *cluster.Cluster, err error)

GetCluster gets the venice cluster object

#### func (*TestBed) GetEndpoint

    func (tb *TestBed) GetEndpoint(meta *api.ObjectMeta) (ep *workload.Endpoint, err error)

GetEndpoint returns the endpoint

#### func (*TestBed) GetSGPolicy

    func (tb *TestBed) GetSGPolicy(meta *api.ObjectMeta) (sgp *security.SGPolicy, err error)

GetSGPolicy gets SGPolicy from venice cluster

#### func (*TestBed) GetSmartNIC

    func (tb *TestBed) GetSmartNIC(name string) (sn *cluster.SmartNIC, err error)

GetSmartNIC returns venice smartnic object

#### func (*TestBed) GetVeniceNode

    func (tb *TestBed) GetVeniceNode(name string) (n *cluster.Node, err error)

GetVeniceNode gets venice node state from venice cluster

#### func (*TestBed) GetVeniceURL

    func (tb *TestBed) GetVeniceURL() []string

GetVeniceURL returns venice URL for the testbed

#### func (*TestBed) GetWorkload

    func (tb *TestBed) GetWorkload(meta *api.ObjectMeta) (w *workload.Workload, err error)

GetWorkload returns venice workload by object meta

#### func (*TestBed) HasNaplesHW

    func (tb *TestBed) HasNaplesHW() bool

HasNaplesHW returns true if testbed has Naples HW

#### func (*TestBed) HasNaplesSim

    func (tb *TestBed) HasNaplesSim() bool

HasNaplesSim returns true if testbed is a Naples sim testbed

#### func (*TestBed) InitVeniceConfig

    func (tb *TestBed) InitVeniceConfig() error

InitVeniceConfig initializes base configuration for venice

#### func (*TestBed) ListSmartNIC

    func (tb *TestBed) ListSmartNIC() (snl []*cluster.SmartNIC, err error)

ListSmartNIC gets a list of smartnics

#### func (*TestBed) MakeVeniceCluster

    func (tb *TestBed) MakeVeniceCluster() error

MakeVeniceCluster inits the venice cluster

#### func (*TestBed) NewTrigger

    func (tb *TestBed) NewTrigger() *Trigger

NewTrigger returns a new trigger instance

#### func (*TestBed) SetupAuth

    func (tb *TestBed) SetupAuth(userID, password string) error

SetupAuth bootstraps default tenant, authentication policy, local user and super
admin role

#### func (*TestBed) SetupConfig

    func (tb *TestBed) SetupConfig() error

SetupConfig sets up the venice cluster and basic config (like auth etc)

#### func (*TestBed) SetupVeniceNodes

    func (tb *TestBed) SetupVeniceNodes() error

SetupVeniceNodes sets up some test tools on venice nodes

#### func (*TestBed) UpdateSGPolicy

    func (tb *TestBed) UpdateSGPolicy(sgp *security.SGPolicy) error

UpdateSGPolicy updates an SG policy

#### func (*TestBed) VeniceLoggedInCtx

    func (tb *TestBed) VeniceLoggedInCtx() (context.Context, error)

VeniceLoggedInCtx returns loggedin context for venice

#### func (*TestBed) VeniceRestClient

    func (tb *TestBed) VeniceRestClient() ([]apiclient.Services, error)

VeniceRestClient returns the REST client for venice

#### func (*TestBed) WaitForVeniceClusterUp

    func (tb *TestBed) WaitForVeniceClusterUp() error

WaitForVeniceClusterUp wait for venice cluster to come up
