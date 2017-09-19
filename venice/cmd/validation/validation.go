package validation

import (
	"fmt"
	"net"
	"reflect"

	"k8s.io/apimachinery/pkg/util/validation/field"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cmd"
)

// ValidateObjectMeta validates the ObjectMeta.
func ValidateObjectMeta(meta *api.ObjectMeta, fldPath *field.Path) field.ErrorList {
	// FIXME: Need to happen for all objects in a common place
	allErrs := field.ErrorList{}

	if meta.Name == "" {
		allErrs = append(allErrs, field.Invalid(fldPath.Child("name"), meta.Name, "name is mandatory"))
	}

	return allErrs
}

// ValidateClusterSpecQuorumNodes validates the quorumNodes configuration in cluster spec.
func ValidateClusterSpecQuorumNodes(nodes []string, fldPath *field.Path) field.ErrorList {
	allErrs := field.ErrorList{}

	// Non zero number of nodes.
	if len(nodes) == 0 {
		allErrs = append(allErrs, field.Invalid(fldPath, nodes, "quorum nodes cannot be empty"))
	}

	// No duplicate nodes.
	nodeMap := make(map[string]struct{})
	for ii := range nodes {
		if _, ok := nodeMap[nodes[ii]]; ok {
			allErrs = append(allErrs, field.Invalid(fldPath, nodes, fmt.Sprintf("duplicate quorum node %v", nodes[ii])))
		}
		nodeMap[nodes[ii]] = struct{}{}
	}

	return allErrs
}

// ValidateClusterSpec validates the cluster specification.
func ValidateClusterSpec(spec *cmd.ClusterSpec, fldPath *field.Path) field.ErrorList {
	allErrs := field.ErrorList{}
	if spec == nil {
		return allErrs
	}

	// Virtual IP is specified.
	if spec.VirtualIP == "" {
		allErrs = append(allErrs, field.Invalid(fldPath.Child("virtualIP"), spec.VirtualIP, "virtual IP is mandatory"))
	} else {
		ipaddr := net.ParseIP(spec.VirtualIP)
		if ipaddr == nil {
			allErrs = append(allErrs, field.Invalid(fldPath.Child("virtualIP"), spec.VirtualIP, "virtual IP is invalid"))
		} else if !ipaddr.IsGlobalUnicast() {
			allErrs = append(allErrs, field.Invalid(fldPath.Child("virtualIP"), spec.VirtualIP, "virtual IP is not global unicast address"))
		}
	}

	// Validate specified quorum nodes.
	allErrs = append(allErrs, ValidateClusterSpecQuorumNodes(spec.QuorumNodes, fldPath.Child("quorumNodes"))...)

	return allErrs
}

// ValidateCluster validates the cluster object.
func ValidateCluster(cluster *cmd.Cluster) field.ErrorList {
	allErrs := ValidateObjectMeta(&cluster.ObjectMeta, field.NewPath("metadata"))
	allErrs = append(allErrs, ValidateClusterSpec(&cluster.Spec, field.NewPath("spec"))...)
	if !reflect.DeepEqual(cluster.Status, cmd.ClusterStatus{}) {
		allErrs = append(allErrs, field.Invalid(field.NewPath("status"), cluster.Status, "cluster status must be empty"))
	}
	return allErrs
}
