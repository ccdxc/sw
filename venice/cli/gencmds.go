// (c) Pensando Systems, Inc.
// This is a generated file, edit at risk of loosing it !!

package vcli

import (
	"fmt"
	"strings"

	"github.com/urfave/cli"
)

var editCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		Usage:        "edit a SGPolicy",
		ArgsUsage:    "[SGPolicy]",
		Action:       editCmd,
		BashComplete: bashEditSGPolicyCompleter,
		Flags:        append(editFlags, CreateSGPolicyFlags...),
	},

	{
		Name:         "cluster",
		Usage:        "edit a cluster",
		ArgsUsage:    "[cluster]",
		Action:       editCmd,
		BashComplete: bashEditClusterCompleter,
		Flags:        append(editFlags, CreateClusterFlags...),
	},

	{
		Name:         "endpoint",
		Usage:        "edit a endpoint",
		ArgsUsage:    "[endpoint]",
		Action:       editCmd,
		BashComplete: bashEditEndpointCompleter,
		Flags:        append(editFlags, CreateEndpointFlags...),
	},

	{
		Name:         "lbPolicy",
		Usage:        "edit a lbPolicy",
		ArgsUsage:    "[lbPolicy]",
		Action:       editCmd,
		BashComplete: bashEditLbPolicyCompleter,
		Flags:        append(editFlags, CreateLbPolicyFlags...),
	},

	{
		Name:         "network",
		Usage:        "edit a network",
		ArgsUsage:    "[network]",
		Action:       editCmd,
		BashComplete: bashEditNetworkCompleter,
		Flags:        append(editFlags, CreateNetworkFlags...),
	},

	{
		Name:         "node",
		Usage:        "edit a node",
		ArgsUsage:    "[node]",
		Action:       editCmd,
		BashComplete: bashEditNodeCompleter,
		Flags:        append(editFlags, CreateNodeFlags...),
	},

	{
		Name:         "permission",
		Usage:        "edit a permission",
		ArgsUsage:    "[permission]",
		Action:       editCmd,
		BashComplete: bashEditPermissionCompleter,
		Flags:        append(editFlags, CreatePermissionFlags...),
	},

	{
		Name:         "role",
		Usage:        "edit a role",
		ArgsUsage:    "[role]",
		Action:       editCmd,
		BashComplete: bashEditRoleCompleter,
		Flags:        append(editFlags, CreateRoleFlags...),
	},

	{
		Name:         "securityGroup",
		Usage:        "edit a securityGroup",
		ArgsUsage:    "[securityGroup]",
		Action:       editCmd,
		BashComplete: bashEditSecurityGroupCompleter,
		Flags:        append(editFlags, CreateSecurityGroupFlags...),
	},

	{
		Name:         "service",
		Usage:        "edit a service",
		ArgsUsage:    "[service]",
		Action:       editCmd,
		BashComplete: bashEditServiceCompleter,
		Flags:        append(editFlags, CreateServiceFlags...),
	},

	{
		Name:         "smartNIC",
		Usage:        "edit a smartNIC",
		ArgsUsage:    "[smartNIC]",
		Action:       editCmd,
		BashComplete: bashEditSmartNICCompleter,
		Flags:        append(editFlags, CreateSmartNICFlags...),
	},

	{
		Name:         "tenant",
		Usage:        "edit a tenant",
		ArgsUsage:    "[tenant]",
		Action:       editCmd,
		BashComplete: bashEditTenantCompleter,
		Flags:        append(editFlags, CreateTenantFlags...),
	},

	{
		Name:         "user",
		Usage:        "edit a user",
		ArgsUsage:    "[user]",
		Action:       editCmd,
		BashComplete: bashEditUserCompleter,
		Flags:        append(editFlags, CreateUserFlags...),
	},
}

var updateCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		Usage:        "create a SGPolicy",
		ArgsUsage:    "[SGPolicy]",
		Action:       updateCmd,
		BashComplete: bashUpdateSGPolicyCompleter,
		Flags:        append(createFlags, CreateSGPolicyFlags...),
	},

	{
		Name:         "cluster",
		Usage:        "create a cluster",
		ArgsUsage:    "[cluster]",
		Action:       updateCmd,
		BashComplete: bashUpdateClusterCompleter,
		Flags:        append(createFlags, CreateClusterFlags...),
	},

	{
		Name:         "endpoint",
		Usage:        "create a endpoint",
		ArgsUsage:    "[endpoint]",
		Action:       updateCmd,
		BashComplete: bashUpdateEndpointCompleter,
		Flags:        append(createFlags, CreateEndpointFlags...),
	},

	{
		Name:         "lbPolicy",
		Usage:        "create a lbPolicy",
		ArgsUsage:    "[lbPolicy]",
		Action:       updateCmd,
		BashComplete: bashUpdateLbPolicyCompleter,
		Flags:        append(createFlags, CreateLbPolicyFlags...),
	},

	{
		Name:         "network",
		Usage:        "create a network",
		ArgsUsage:    "[network]",
		Action:       updateCmd,
		BashComplete: bashUpdateNetworkCompleter,
		Flags:        append(createFlags, CreateNetworkFlags...),
	},

	{
		Name:         "node",
		Usage:        "create a node",
		ArgsUsage:    "[node]",
		Action:       updateCmd,
		BashComplete: bashUpdateNodeCompleter,
		Flags:        append(createFlags, CreateNodeFlags...),
	},

	{
		Name:         "permission",
		Usage:        "create a permission",
		ArgsUsage:    "[permission]",
		Action:       updateCmd,
		BashComplete: bashUpdatePermissionCompleter,
		Flags:        append(createFlags, CreatePermissionFlags...),
	},

	{
		Name:         "role",
		Usage:        "create a role",
		ArgsUsage:    "[role]",
		Action:       updateCmd,
		BashComplete: bashUpdateRoleCompleter,
		Flags:        append(createFlags, CreateRoleFlags...),
	},

	{
		Name:         "securityGroup",
		Usage:        "create a securityGroup",
		ArgsUsage:    "[securityGroup]",
		Action:       updateCmd,
		BashComplete: bashUpdateSecurityGroupCompleter,
		Flags:        append(createFlags, CreateSecurityGroupFlags...),
	},

	{
		Name:         "service",
		Usage:        "create a service",
		ArgsUsage:    "[service]",
		Action:       updateCmd,
		BashComplete: bashUpdateServiceCompleter,
		Flags:        append(createFlags, CreateServiceFlags...),
	},

	{
		Name:         "smartNIC",
		Usage:        "create a smartNIC",
		ArgsUsage:    "[smartNIC]",
		Action:       updateCmd,
		BashComplete: bashUpdateSmartNICCompleter,
		Flags:        append(createFlags, CreateSmartNICFlags...),
	},

	{
		Name:         "tenant",
		Usage:        "create a tenant",
		ArgsUsage:    "[tenant]",
		Action:       updateCmd,
		BashComplete: bashUpdateTenantCompleter,
		Flags:        append(createFlags, CreateTenantFlags...),
	},

	{
		Name:         "user",
		Usage:        "create a user",
		ArgsUsage:    "[user]",
		Action:       updateCmd,
		BashComplete: bashUpdateUserCompleter,
		Flags:        append(createFlags, CreateUserFlags...),
	},
}

var createCommands = []cli.Command{
	{
		Name:         "upload",
		Action:       createFromFile,
		BashComplete: bashFileNameCompleter,
		Usage:        "Specify comma separated file names, directory names or URLs",
		ArgsUsage:    "[filename]",
	},

	{
		Name:         "SGPolicy",
		Usage:        "create a SGPolicy",
		ArgsUsage:    "[SGPolicy]",
		Action:       createCmd,
		BashComplete: bashCreateSGPolicyCompleter,
		Flags:        append(createFlags, CreateSGPolicyFlags...),
	},

	{
		Name:         "endpoint",
		Usage:        "create a endpoint",
		ArgsUsage:    "[endpoint]",
		Action:       createCmd,
		BashComplete: bashCreateEndpointCompleter,
		Flags:        append(createFlags, CreateEndpointFlags...),
	},

	{
		Name:         "lbPolicy",
		Usage:        "create a lbPolicy",
		ArgsUsage:    "[lbPolicy]",
		Action:       createCmd,
		BashComplete: bashCreateLbPolicyCompleter,
		Flags:        append(createFlags, CreateLbPolicyFlags...),
	},

	{
		Name:         "network",
		Usage:        "create a network",
		ArgsUsage:    "[network]",
		Action:       createCmd,
		BashComplete: bashCreateNetworkCompleter,
		Flags:        append(createFlags, CreateNetworkFlags...),
	},

	{
		Name:         "node",
		Usage:        "create a node",
		ArgsUsage:    "[node]",
		Action:       createCmd,
		BashComplete: bashCreateNodeCompleter,
		Flags:        append(createFlags, CreateNodeFlags...),
	},

	{
		Name:         "permission",
		Usage:        "create a permission",
		ArgsUsage:    "[permission]",
		Action:       createCmd,
		BashComplete: bashCreatePermissionCompleter,
		Flags:        append(createFlags, CreatePermissionFlags...),
	},

	{
		Name:         "role",
		Usage:        "create a role",
		ArgsUsage:    "[role]",
		Action:       createCmd,
		BashComplete: bashCreateRoleCompleter,
		Flags:        append(createFlags, CreateRoleFlags...),
	},

	{
		Name:         "securityGroup",
		Usage:        "create a securityGroup",
		ArgsUsage:    "[securityGroup]",
		Action:       createCmd,
		BashComplete: bashCreateSecurityGroupCompleter,
		Flags:        append(createFlags, CreateSecurityGroupFlags...),
	},

	{
		Name:         "service",
		Usage:        "create a service",
		ArgsUsage:    "[service]",
		Action:       createCmd,
		BashComplete: bashCreateServiceCompleter,
		Flags:        append(createFlags, CreateServiceFlags...),
	},

	{
		Name:         "smartNIC",
		Usage:        "create a smartNIC",
		ArgsUsage:    "[smartNIC]",
		Action:       createCmd,
		BashComplete: bashCreateSmartNICCompleter,
		Flags:        append(createFlags, CreateSmartNICFlags...),
	},

	{
		Name:         "tenant",
		Usage:        "create a tenant",
		ArgsUsage:    "[tenant]",
		Action:       createCmd,
		BashComplete: bashCreateTenantCompleter,
		Flags:        append(createFlags, CreateTenantFlags...),
	},

	{
		Name:         "user",
		Usage:        "create a user",
		ArgsUsage:    "[user]",
		Action:       createCmd,
		BashComplete: bashCreateUserCompleter,
		Flags:        append(createFlags, CreateUserFlags...),
	},
}

var readCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		ArgsUsage:    " ",
		Usage:        "read specified [SGPolicy](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadSGPolicyCompleter,
	},

	{
		Name:         "cluster",
		ArgsUsage:    " ",
		Usage:        "read specified [cluster](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadClusterCompleter,
	},

	{
		Name:         "endpoint",
		ArgsUsage:    " ",
		Usage:        "read specified [endpoint](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadEndpointCompleter,
	},

	{
		Name:         "lbPolicy",
		ArgsUsage:    " ",
		Usage:        "read specified [lbPolicy](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadLbPolicyCompleter,
	},

	{
		Name:         "network",
		ArgsUsage:    " ",
		Usage:        "read specified [network](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadNetworkCompleter,
	},

	{
		Name:         "node",
		ArgsUsage:    " ",
		Usage:        "read specified [node](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadNodeCompleter,
	},

	{
		Name:         "permission",
		ArgsUsage:    " ",
		Usage:        "read specified [permission](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadPermissionCompleter,
	},

	{
		Name:         "role",
		ArgsUsage:    " ",
		Usage:        "read specified [role](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadRoleCompleter,
	},

	{
		Name:         "securityGroup",
		ArgsUsage:    " ",
		Usage:        "read specified [securityGroup](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadSecurityGroupCompleter,
	},

	{
		Name:         "service",
		ArgsUsage:    " ",
		Usage:        "read specified [service](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadServiceCompleter,
	},

	{
		Name:         "smartNIC",
		ArgsUsage:    " ",
		Usage:        "read specified [smartNIC](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadSmartNICCompleter,
	},

	{
		Name:         "tenant",
		ArgsUsage:    " ",
		Usage:        "read specified [tenant](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadTenantCompleter,
	},

	{
		Name:         "user",
		ArgsUsage:    " ",
		Usage:        "read specified [user](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashReadUserCompleter,
	},
}

var deleteCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		ArgsUsage:    "[SGPolicy]",
		Usage:        "delete specified [SGPolicy](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteSGPolicyCompleter,
	},

	{
		Name:         "cluster",
		ArgsUsage:    "[cluster]",
		Usage:        "delete specified [cluster](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteClusterCompleter,
	},

	{
		Name:         "endpoint",
		ArgsUsage:    "[endpoint]",
		Usage:        "delete specified [endpoint](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteEndpointCompleter,
	},

	{
		Name:         "lbPolicy",
		ArgsUsage:    "[lbPolicy]",
		Usage:        "delete specified [lbPolicy](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteLbPolicyCompleter,
	},

	{
		Name:         "network",
		ArgsUsage:    "[network]",
		Usage:        "delete specified [network](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteNetworkCompleter,
	},

	{
		Name:         "node",
		ArgsUsage:    "[node]",
		Usage:        "delete specified [node](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteNodeCompleter,
	},

	{
		Name:         "permission",
		ArgsUsage:    "[permission]",
		Usage:        "delete specified [permission](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeletePermissionCompleter,
	},

	{
		Name:         "role",
		ArgsUsage:    "[role]",
		Usage:        "delete specified [role](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteRoleCompleter,
	},

	{
		Name:         "securityGroup",
		ArgsUsage:    "[securityGroup]",
		Usage:        "delete specified [securityGroup](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteSecurityGroupCompleter,
	},

	{
		Name:         "service",
		ArgsUsage:    "[service]",
		Usage:        "delete specified [service](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteServiceCompleter,
	},

	{
		Name:         "smartNIC",
		ArgsUsage:    "[smartNIC]",
		Usage:        "delete specified [smartNIC](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteSmartNICCompleter,
	},

	{
		Name:         "tenant",
		ArgsUsage:    "[tenant]",
		Usage:        "delete specified [tenant](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteTenantCompleter,
	},

	{
		Name:         "user",
		ArgsUsage:    "[user]",
		Usage:        "delete specified [user](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDeleteUserCompleter,
	},
}

var labelCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		ArgsUsage:    " ",
		Usage:        "label specified [SGPolicy](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelSGPolicyCompleter,
	},

	{
		Name:         "cluster",
		ArgsUsage:    " ",
		Usage:        "label specified [cluster](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelClusterCompleter,
	},

	{
		Name:         "endpoint",
		ArgsUsage:    " ",
		Usage:        "label specified [endpoint](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelEndpointCompleter,
	},

	{
		Name:         "lbPolicy",
		ArgsUsage:    " ",
		Usage:        "label specified [lbPolicy](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelLbPolicyCompleter,
	},

	{
		Name:         "network",
		ArgsUsage:    " ",
		Usage:        "label specified [network](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelNetworkCompleter,
	},

	{
		Name:         "node",
		ArgsUsage:    " ",
		Usage:        "label specified [node](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelNodeCompleter,
	},

	{
		Name:         "permission",
		ArgsUsage:    " ",
		Usage:        "label specified [permission](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelPermissionCompleter,
	},

	{
		Name:         "role",
		ArgsUsage:    " ",
		Usage:        "label specified [role](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelRoleCompleter,
	},

	{
		Name:         "securityGroup",
		ArgsUsage:    " ",
		Usage:        "label specified [securityGroup](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelSecurityGroupCompleter,
	},

	{
		Name:         "service",
		ArgsUsage:    " ",
		Usage:        "label specified [service](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelServiceCompleter,
	},

	{
		Name:         "smartNIC",
		ArgsUsage:    " ",
		Usage:        "label specified [smartNIC](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelSmartNICCompleter,
	},

	{
		Name:         "tenant",
		ArgsUsage:    " ",
		Usage:        "label specified [tenant](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelTenantCompleter,
	},

	{
		Name:         "user",
		ArgsUsage:    " ",
		Usage:        "label specified [user](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabelUserCompleter,
	},
}

var exampleCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		Usage:        "show examples of the SGPolicy",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleSGPolicyCompleter,
	},

	{
		Name:         "cluster",
		Usage:        "show examples of the cluster",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleClusterCompleter,
	},

	{
		Name:         "endpoint",
		Usage:        "show examples of the endpoint",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleEndpointCompleter,
	},

	{
		Name:         "lbPolicy",
		Usage:        "show examples of the lbPolicy",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleLbPolicyCompleter,
	},

	{
		Name:         "network",
		Usage:        "show examples of the network",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleNetworkCompleter,
	},

	{
		Name:         "node",
		Usage:        "show examples of the node",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleNodeCompleter,
	},

	{
		Name:         "permission",
		Usage:        "show examples of the permission",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExamplePermissionCompleter,
	},

	{
		Name:         "role",
		Usage:        "show examples of the role",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleRoleCompleter,
	},

	{
		Name:         "securityGroup",
		Usage:        "show examples of the securityGroup",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleSecurityGroupCompleter,
	},

	{
		Name:         "service",
		Usage:        "show examples of the service",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleServiceCompleter,
	},

	{
		Name:         "smartNIC",
		Usage:        "show examples of the smartNIC",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleSmartNICCompleter,
	},

	{
		Name:         "tenant",
		Usage:        "show examples of the tenant",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleTenantCompleter,
	},

	{
		Name:         "user",
		Usage:        "show examples of the user",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExampleUserCompleter,
	},
}

var definitionCommands = []cli.Command{

	{
		Name:         "SGPolicy",
		Usage:        "show definifition of the SGPolicy",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionSGPolicyCompleter,
	},

	{
		Name:         "cluster",
		Usage:        "show definifition of the cluster",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionClusterCompleter,
	},

	{
		Name:         "endpoint",
		Usage:        "show definifition of the endpoint",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionEndpointCompleter,
	},

	{
		Name:         "lbPolicy",
		Usage:        "show definifition of the lbPolicy",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionLbPolicyCompleter,
	},

	{
		Name:         "network",
		Usage:        "show definifition of the network",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionNetworkCompleter,
	},

	{
		Name:         "node",
		Usage:        "show definifition of the node",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionNodeCompleter,
	},

	{
		Name:         "permission",
		Usage:        "show definifition of the permission",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionPermissionCompleter,
	},

	{
		Name:         "role",
		Usage:        "show definifition of the role",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionRoleCompleter,
	},

	{
		Name:         "securityGroup",
		Usage:        "show definifition of the securityGroup",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionSecurityGroupCompleter,
	},

	{
		Name:         "service",
		Usage:        "show definifition of the service",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionServiceCompleter,
	},

	{
		Name:         "smartNIC",
		Usage:        "show definifition of the smartNIC",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionSmartNICCompleter,
	},

	{
		Name:         "tenant",
		Usage:        "show definifition of the tenant",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionTenantCompleter,
	},

	{
		Name:         "user",
		Usage:        "show definifition of the user",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinitionUserCompleter,
	},
}

// CreateSGPolicyFlags specifies flagsfor SGPolicy create operation
var CreateSGPolicyFlags = []cli.Flag{

	cli.StringSliceFlag{
		Name: "action",
	},

	cli.StringSliceFlag{
		Name: "apps",
	},

	cli.StringSliceFlag{
		Name: "attach-groups",
	},

	cli.BoolFlag{
		Name: "attach-tenant",
	},

	cli.StringSliceFlag{
		Name: "from-ip-addresses",
	},

	cli.StringSliceFlag{
		Name: "from-security-groups",
	},

	cli.StringSliceFlag{
		Name: "to-ip-addresses",
	},

	cli.StringSliceFlag{
		Name: "to-security-groups",
	},
}

// CreateClusterFlags specifies flagsfor cluster create operation
var CreateClusterFlags = []cli.Flag{

	cli.BoolFlag{
		Name: "auto-admit-nics",
	},

	cli.StringSliceFlag{
		Name: "ntp-servers",
	},

	cli.StringSliceFlag{
		Name: "quorum-nodes",
	},

	cli.StringFlag{
		Name: "virtual-ip",
	},
}

// CreateEndpointFlags specifies flagsfor endpoint create operation
var CreateEndpointFlags = []cli.Flag{}

// CreateLbPolicyFlags specifies flagsfor lbPolicy create operation
var CreateLbPolicyFlags = []cli.Flag{

	cli.StringFlag{
		Name: "algorithm",
	},

	cli.UintFlag{
		Name: "declare-healthy-count",
	},

	cli.UintFlag{
		Name: "interval",
	},

	cli.UintFlag{
		Name: "max-timeouts",
	},

	cli.StringFlag{
		Name: "probe-port-or-url",
	},

	cli.UintFlag{
		Name: "probes-per-interval",
	},

	cli.StringFlag{
		Name: "session-affinity",
	},

	cli.StringFlag{
		Name: "type",
	},
}

// CreateNetworkFlags specifies flagsfor network create operation
var CreateNetworkFlags = []cli.Flag{

	cli.StringFlag{
		Name: "ipv4-gateway",
	},

	cli.StringFlag{
		Name: "ipv4-subnet",
	},

	cli.StringFlag{
		Name: "ipv6-gateway",
	},

	cli.StringFlag{
		Name: "ipv6-subnet",
	},

	cli.StringFlag{
		Name: "type",
	},

	cli.UintFlag{
		Name: "vlan-id",
	},

	cli.UintFlag{
		Name: "vxlan-vni",
	},
}

// CreateNodeFlags specifies flagsfor node create operation
var CreateNodeFlags = []cli.Flag{}

// CreatePermissionFlags specifies flagsfor permission create operation
var CreatePermissionFlags = []cli.Flag{

	cli.StringFlag{
		Name: "action",
	},

	cli.StringFlag{
		Name: "objectSelector",
	},

	cli.StringFlag{
		Name: "validUntil",
	},
}

// CreateRoleFlags specifies flagsfor role create operation
var CreateRoleFlags = []cli.Flag{

	cli.StringSliceFlag{
		Name: "permissions",
	},
}

// CreateSecurityGroupFlags specifies flagsfor securityGroup create operation
var CreateSecurityGroupFlags = []cli.Flag{

	cli.StringSliceFlag{
		Name: "match-prefixes",
	},

	cli.StringSliceFlag{
		Name: "service-labels",
	},

	cli.StringFlag{
		Name: "workload-selector",
	},
}

// CreateServiceFlags specifies flagsfor service create operation
var CreateServiceFlags = []cli.Flag{

	cli.StringFlag{
		Name: "client-authentication",
	},

	cli.StringFlag{
		Name: "lb-policy",
	},

	cli.StringFlag{
		Name: "ports",
	},

	cli.StringSliceFlag{
		Name: "tls-client-allowed-peer-id",
	},

	cli.StringFlag{
		Name: "tls-client-certificates-selector",
	},

	cli.StringSliceFlag{
		Name: "tls-client-trust-roots",
	},

	cli.StringSliceFlag{
		Name: "tls-server-allowed-peer-id",
	},

	cli.StringSliceFlag{
		Name: "tls-server-certificates",
	},

	cli.StringSliceFlag{
		Name: "tls-server-trust-roots",
	},

	cli.StringFlag{
		Name: "virtual-ip",
	},

	cli.StringSliceFlag{
		Name: "workload-labels",
	},
}

// CreateSmartNICFlags specifies flagsfor smartNIC create operation
var CreateSmartNICFlags = []cli.Flag{

	cli.BoolFlag{
		Name: "admit",
	},

	cli.StringSliceFlag{
		Name: "controllers",
	},

	cli.StringFlag{
		Name: "default-gw",
	},

	cli.StringSliceFlag{
		Name: "dns-servers",
	},

	cli.StringFlag{
		Name: "hostname",
	},

	cli.StringFlag{
		Name: "ip-address",
	},

	cli.StringFlag{
		Name: "mgmt-mode",
	},

	cli.UintFlag{
		Name: "mgmt-vlan",
	},
}

// CreateTenantFlags specifies flagsfor tenant create operation
var CreateTenantFlags = []cli.Flag{

	cli.StringFlag{
		Name: "admin-user",
	},
}

// CreateUserFlags specifies flagsfor user create operation
var CreateUserFlags = []cli.Flag{

	cli.StringSliceFlag{
		Name: "roles",
	},
}

func bashExampleSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectSGPolicyCompleter(c)
}

func bashReadSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectSGPolicyCompleter(c)
}

func bashDeleteSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectSGPolicyCompleter(c)
}

func bashObjectSGPolicyCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "SGPolicy"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-SGPolicys! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectSGPolicyCompleter(c)
}

func bashCreateSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateSGPolicyFlags...))

	if _, found := getLastFlagSuggestion(c, CreateSGPolicyFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{SGPolicy} ")
	}
}

func bashUpdateSGPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateSGPolicyFlags...))

	if _, found := getLastFlagSuggestion(c, CreateSGPolicyFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectSGPolicyCompleter(c)
	}
}

func bashExampleClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectClusterCompleter(c)
}

func bashReadClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectClusterCompleter(c)
}

func bashDeleteClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectClusterCompleter(c)
}

func bashObjectClusterCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "cluster"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-clusters! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectClusterCompleter(c)
}

func bashCreateClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateClusterFlags...))

	if _, found := getLastFlagSuggestion(c, CreateClusterFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{cluster} ")
	}
}

func bashUpdateClusterCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateClusterFlags...))

	if _, found := getLastFlagSuggestion(c, CreateClusterFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectClusterCompleter(c)
	}
}

func bashExampleEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectEndpointCompleter(c)
}

func bashReadEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectEndpointCompleter(c)
}

func bashDeleteEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectEndpointCompleter(c)
}

func bashObjectEndpointCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "endpoint"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-endpoints! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectEndpointCompleter(c)
}

func bashCreateEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateEndpointFlags...))

	if _, found := getLastFlagSuggestion(c, CreateEndpointFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{endpoint} ")
	}
}

func bashUpdateEndpointCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateEndpointFlags...))

	if _, found := getLastFlagSuggestion(c, CreateEndpointFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectEndpointCompleter(c)
	}
}

func bashExampleLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectLbPolicyCompleter(c)
}

func bashReadLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectLbPolicyCompleter(c)
}

func bashDeleteLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectLbPolicyCompleter(c)
}

func bashObjectLbPolicyCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "lbPolicy"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-lbPolicys! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectLbPolicyCompleter(c)
}

func bashCreateLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateLbPolicyFlags...))

	if _, found := getLastFlagSuggestion(c, CreateLbPolicyFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{lbPolicy} ")
	}
}

func bashUpdateLbPolicyCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateLbPolicyFlags...))

	if _, found := getLastFlagSuggestion(c, CreateLbPolicyFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectLbPolicyCompleter(c)
	}
}

func bashExampleNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectNetworkCompleter(c)
}

func bashReadNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectNetworkCompleter(c)
}

func bashDeleteNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectNetworkCompleter(c)
}

func bashObjectNetworkCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "network"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-networks! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectNetworkCompleter(c)
}

func bashCreateNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateNetworkFlags...))

	if _, found := getLastFlagSuggestion(c, CreateNetworkFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{network} ")
	}
}

func bashUpdateNetworkCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateNetworkFlags...))

	if _, found := getLastFlagSuggestion(c, CreateNetworkFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectNetworkCompleter(c)
	}
}

func bashExampleNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectNodeCompleter(c)
}

func bashReadNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectNodeCompleter(c)
}

func bashDeleteNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectNodeCompleter(c)
}

func bashObjectNodeCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "node"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-nodes! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectNodeCompleter(c)
}

func bashCreateNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateNodeFlags...))

	if _, found := getLastFlagSuggestion(c, CreateNodeFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{node} ")
	}
}

func bashUpdateNodeCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateNodeFlags...))

	if _, found := getLastFlagSuggestion(c, CreateNodeFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectNodeCompleter(c)
	}
}

func bashExamplePermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionPermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelPermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectPermissionCompleter(c)
}

func bashReadPermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectPermissionCompleter(c)
}

func bashDeletePermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectPermissionCompleter(c)
}

func bashObjectPermissionCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "permission"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-permissions! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditPermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectPermissionCompleter(c)
}

func bashCreatePermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreatePermissionFlags...))

	if _, found := getLastFlagSuggestion(c, CreatePermissionFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{permission} ")
	}
}

func bashUpdatePermissionCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreatePermissionFlags...))

	if _, found := getLastFlagSuggestion(c, CreatePermissionFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectPermissionCompleter(c)
	}
}

func bashExampleRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectRoleCompleter(c)
}

func bashReadRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectRoleCompleter(c)
}

func bashDeleteRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectRoleCompleter(c)
}

func bashObjectRoleCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "role"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-roles! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectRoleCompleter(c)
}

func bashCreateRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateRoleFlags...))

	if _, found := getLastFlagSuggestion(c, CreateRoleFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{role} ")
	}
}

func bashUpdateRoleCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateRoleFlags...))

	if _, found := getLastFlagSuggestion(c, CreateRoleFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectRoleCompleter(c)
	}
}

func bashExampleSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectSecurityGroupCompleter(c)
}

func bashReadSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectSecurityGroupCompleter(c)
}

func bashDeleteSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectSecurityGroupCompleter(c)
}

func bashObjectSecurityGroupCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "securityGroup"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-securityGroups! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectSecurityGroupCompleter(c)
}

func bashCreateSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateSecurityGroupFlags...))

	if _, found := getLastFlagSuggestion(c, CreateSecurityGroupFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{securityGroup} ")
	}
}

func bashUpdateSecurityGroupCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateSecurityGroupFlags...))

	if _, found := getLastFlagSuggestion(c, CreateSecurityGroupFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectSecurityGroupCompleter(c)
	}
}

func bashExampleServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectServiceCompleter(c)
}

func bashReadServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectServiceCompleter(c)
}

func bashDeleteServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectServiceCompleter(c)
}

func bashObjectServiceCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "service"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-services! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectServiceCompleter(c)
}

func bashCreateServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateServiceFlags...))

	if _, found := getLastFlagSuggestion(c, CreateServiceFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{service} ")
	}
}

func bashUpdateServiceCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateServiceFlags...))

	if _, found := getLastFlagSuggestion(c, CreateServiceFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectServiceCompleter(c)
	}
}

func bashExampleSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectSmartNICCompleter(c)
}

func bashReadSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectSmartNICCompleter(c)
}

func bashDeleteSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectSmartNICCompleter(c)
}

func bashObjectSmartNICCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "smartNIC"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-smartNICs! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectSmartNICCompleter(c)
}

func bashCreateSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateSmartNICFlags...))

	if _, found := getLastFlagSuggestion(c, CreateSmartNICFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{smartNIC} ")
	}
}

func bashUpdateSmartNICCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateSmartNICFlags...))

	if _, found := getLastFlagSuggestion(c, CreateSmartNICFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectSmartNICCompleter(c)
	}
}

func bashExampleTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectTenantCompleter(c)
}

func bashReadTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectTenantCompleter(c)
}

func bashDeleteTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectTenantCompleter(c)
}

func bashObjectTenantCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "tenant"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-tenants! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectTenantCompleter(c)
}

func bashCreateTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateTenantFlags...))

	if _, found := getLastFlagSuggestion(c, CreateTenantFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{tenant} ")
	}
}

func bashUpdateTenantCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateTenantFlags...))

	if _, found := getLastFlagSuggestion(c, CreateTenantFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectTenantCompleter(c)
	}
}

func bashExampleUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinitionUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabelUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, labelFlags)
	bashObjectUserCompleter(c)
}

func bashReadUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, readFlags)
	bashObjectUserCompleter(c)
}

func bashDeleteUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjectUserCompleter(c)
}

func bashObjectUserCompleter(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "user"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-users! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEditUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, editFlags)
	bashObjectUserCompleter(c)
}

func bashCreateUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateUserFlags...))

	if _, found := getLastFlagSuggestion(c, CreateUserFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{user} ")
	}
}

func bashUpdateUserCompleter(c *cli.Context) {
	BashCompleter(c, []cli.Command{}, append(createFlags, CreateUserFlags...))

	if _, found := getLastFlagSuggestion(c, CreateUserFlags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObjectUserCompleter(c)
	}
}
