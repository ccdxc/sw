package ops

import (
	"fmt"
	"net"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/errors"
)

// makeQuorumConfig makes etcd quorum configuration for the specified quorum nodes.
func makeQuorumConfig(uuid string, nodes []string) (*grpc.QuorumConfig, error) {
	quorumMembers := make([]*grpc.QuorumMember, 0)
	for ii := 0; ii < len(nodes); ii++ {
		addrs, err := net.LookupHost(nodes[ii])
		if err != nil {
			return nil, errors.NewBadRequest(fmt.Sprintf("Failed to lookup host %v, error: %v", nodes[ii], err))
		}
		quorumMembers = append(quorumMembers, &grpc.QuorumMember{
			Name:       nodes[ii],
			PeerUrls:   []string{fmt.Sprintf("http://%s:%s", addrs[0], env.Options.KVStore.PeerPort)},
			ClientUrls: []string{fmt.Sprintf("http://%s:%s", addrs[0], env.Options.KVStore.ClientPort)},
		})
	}

	return &grpc.QuorumConfig{
		Id:            uuid,
		QuorumMembers: quorumMembers,
	}, nil
}
