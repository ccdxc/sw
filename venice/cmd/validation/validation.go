package validation

import (
	"context"
	"fmt"
	"net"
	"reflect"
	"time"

	"k8s.io/apimachinery/pkg/util/validation"
	"k8s.io/apimachinery/pkg/util/validation/field"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
)

var (
	resolverTimeout = 5 * time.Second
)

// ResolverInterface is the interface that validator uses to resolve names
type ResolverInterface interface {
	LookupHost(ctx context.Context, host string) (addrs []string, err error)
}

// IsValidNodeIP returns true if the supplied IP address is valid for a cluster node.
// IsGlobalUnicast returns (!ip.Equal(IPv4bcast) && !ip.IsUnspecified() && !ip.IsLoopback() && !ip.IsMulticast() && !ip.IsLinkLocalUnicast())
func IsValidNodeIP(ip *net.IP) bool {
	return ip.IsGlobalUnicast()
}

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
func ValidateClusterSpecQuorumNodes(resolver ResolverInterface, nodes []string, fldPath *field.Path) field.ErrorList {
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

	// Each quorum node ID must be either a valid DNS name or an IP address
	for _, n := range nodes {
		if ip := net.ParseIP(n); ip != nil {
			// quorum node ID is IP address
			if !IsValidNodeIP(&ip) {
				allErrs = append(allErrs, field.Invalid(fldPath, nodes, fmt.Sprintf("invalid IP address %v for quorum node", n)))
			}
		} else {
			// quorum node ID is a DNS name
			if len(validation.IsDNS1123Subdomain(n)) > 0 {
				allErrs = append(allErrs, field.Invalid(fldPath, nodes, fmt.Sprintf("quorum node %v is not a valid DNS name or IP address", n)))
			}
			// Check that it resolves to exactly 1 IP address
			ctx, cancel := context.WithTimeout(context.Background(), resolverTimeout)
			addrs, err := resolver.LookupHost(ctx, n)
			if err != nil {
				allErrs = append(allErrs, field.Invalid(fldPath, nodes, fmt.Sprintf("Error resolving IP address for node %v: %v", n, err)))
			}
			if len(addrs) == 0 {
				allErrs = append(allErrs, field.Invalid(fldPath, nodes, fmt.Sprintf("DNS did not return an IP address for node %v", n)))
			} else if len(addrs) > 1 {
				allErrs = append(allErrs, field.Invalid(fldPath, nodes, fmt.Sprintf("DNS returned too many IP address for node %v: %v. Exactly 1 is required", n, addrs)))
			}
			cancel()
		}
	}

	return allErrs
}

// ValidateClusterSpec validates the cluster specification.
func ValidateClusterSpec(resolver ResolverInterface, spec *cmd.ClusterSpec, fldPath *field.Path) field.ErrorList {
	allErrs := field.ErrorList{}
	if spec == nil {
		return allErrs
	}

	// Virtual IP is specified.
	if spec.VirtualIP != "" {
		ipaddr := net.ParseIP(spec.VirtualIP)
		if ipaddr == nil {
			allErrs = append(allErrs, field.Invalid(fldPath.Child("virtualIP"), spec.VirtualIP, "virtual IP is invalid"))
		} else if !ipaddr.IsGlobalUnicast() {
			allErrs = append(allErrs, field.Invalid(fldPath.Child("virtualIP"), spec.VirtualIP, "virtual IP is not global unicast address"))
		}
	}

	// Validate specified quorum nodes.
	allErrs = append(allErrs, ValidateClusterSpecQuorumNodes(resolver, spec.QuorumNodes, fldPath.Child("quorumNodes"))...)

	return allErrs
}

// ValidateCluster validates the cluster object.
func ValidateCluster(cluster *cmd.Cluster, resolver ResolverInterface) field.ErrorList {
	allErrs := ValidateObjectMeta(&cluster.ObjectMeta, field.NewPath("metadata"))
	allErrs = append(allErrs, ValidateClusterSpec(resolver, &cluster.Spec, field.NewPath("spec"))...)
	if !reflect.DeepEqual(cluster.Status, cmd.ClusterStatus{}) {
		allErrs = append(allErrs, field.Invalid(field.NewPath("status"), cluster.Status, "cluster status must be empty"))
	}
	return allErrs
}
