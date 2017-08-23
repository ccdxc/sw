package ops

import (
	"context"
	"fmt"

	"github.com/pborman/uuid"
	"github.com/pensando/sw/utils/log"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/cmd/grpc"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/errors"
	"github.com/pensando/sw/utils/rpckit"
	"github.com/pensando/sw/utils/version"
)

// makeNode creates a new Node object.
func makeNode(name string) *cmd.Node {
	return &cmd.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
			UUID: uuid.New(),
		},
	}
}

// response contains grpc response and error for a given host.
type response struct {
	host string
	obj  interface{}
	err  error
}

// clusterClientFn returns a gRPC cluster client.
type clusterClientFn func(host string) (grpc.ClusterClient, error)
type messageFn func(host string, req interface{}) interface{}
type postProcessFn func(host string, obj interface{}) error

// defaultClusterClientFn creates a default gRPC cluster client.
func defaultClusterClientFn(host string) (grpc.ClusterClient, error) {
	rpcClient, err := rpckit.NewRPCClient("cmd", fmt.Sprintf("%s:%s", host, globals.CMDGRPCPort))
	if err != nil {
		log.Errorf("RPC client creation for %v failed with error: %v", host, err)
		return nil, errors.NewInternalError(err)
	}
	return grpc.NewClusterClient(rpcClient.ClientConn), nil
}

// sendByMsgType sends a cluster gRPC message based on message type.
func sendByMsgType(client grpc.ClusterClient, msgType string, req interface{}) (interface{}, error) {
	switch msgType {
	case "prejoin":
		return client.PreJoin(context.Background(), req.(*grpc.ClusterPreJoinReq))
	case "join":
		return client.Join(context.Background(), req.(*grpc.ClusterJoinReq))
	case "disjoin":
		return client.Disjoin(context.Background(), req.(*grpc.ClusterDisjoinReq))
	}
	return nil, fmt.Errorf("Unknown msg type: %v", msgType)
}

// sendClusterGRPCs sends provided gRPC message to provided nodes. mFn is used to modify the message
// per node. ppFn is used to post process the response per node.
func sendClusterGRPCs(cFn clusterClientFn, msgType string, nodes []string, req interface{}, mFn messageFn, ppFn postProcessFn) (map[string]response, error) {
	if cFn == nil {
		cFn = defaultClusterClientFn
	}
	respCh := make(chan response, len(nodes))
	resps := make(map[string]response)

	for ii := range nodes {
		go func(host string, respCh chan<- response) {
			clusterClient, err := cFn(host)
			if err != nil {
				respCh <- response{
					host: host,
					err:  err,
				}
				return
			}

			// modify message (per host) if desired.
			newReq := req
			if mFn != nil {
				newReq = mFn(host, req)
			}

			// timeouts are handled by gRPC and result in an error.
			resp, err := sendByMsgType(clusterClient, msgType, newReq)
			if err != nil {
				respCh <- response{
					host: host,
					err:  err,
				}
				return
			}
			if ppFn != nil {
				err = ppFn(host, resp)
				if err != nil {
					respCh <- response{
						host: host,
						err:  err,
					}
					return
				}
			}
			respCh <- response{
				host: host,
				obj:  resp,
			}
		}(nodes[ii], respCh)
	}

	ii := 0
	for {
		select {
		case resp, ok := <-respCh:
			if !ok {
				log.Errorf("Failed on sending %v cluster gRPCs", msgType)
				return nil, errors.NewInternalError(fmt.Errorf("Failed on sending %v cluster gRPCs", msgType))
			}
			resps[resp.host] = resp
			ii++
		}
		if ii == len(nodes) {
			break
		}
	}
	return resps, nil
}

// sendPreJoins sends prejoin gRPC message to provided nodes.
func sendPreJoins(cFn clusterClientFn, req *grpc.ClusterPreJoinReq, nodes []string) error {
	ppFn := func(host string, obj interface{}) error {
		resp, ok := obj.(*grpc.ClusterPreJoinResp)
		if !ok {
			return fmt.Errorf("Invalid object %v for conversion to ClusterPreJoinResp", obj)
		}
		if resp.SwVersion != version.Version {
			return fmt.Errorf("Version mismatch: expected %v, got %v", version.Version, resp.SwVersion)
		}
		return nil
	}

	resps, err := sendClusterGRPCs(cFn, "prejoin", nodes, req, nil, ppFn)
	if err != nil {
		return err
	}

	errMsg := ""
	for _, resp := range resps {
		if resp.err != nil {
			errMsg += fmt.Sprintf("%v: %v ", resp.host, resp.err.Error())
		}
	}
	if errMsg != "" {
		return fmt.Errorf("Failed nodes: " + errMsg)
	}
	return nil
}

// sendJoins sends join gRPC message to provided nodes.
func sendJoins(cFn clusterClientFn, req *grpc.ClusterJoinReq, nodes []string) error {
	mFn := func(host string, req interface{}) interface{} {
		origReq := req.(*grpc.ClusterJoinReq)
		newReq := *origReq
		found := false
		if newReq.QuorumConfig != nil {
			for ii := range newReq.QuorumConfig.QuorumMembers {
				if host == newReq.QuorumConfig.QuorumMembers[ii].Name {
					found = true
					break
				}
			}
		}
		if !found {
			newReq.QuorumConfig = nil
		}
		return &newReq
	}

	resps, err := sendClusterGRPCs(cFn, "join", nodes, req, mFn, nil)
	if err != nil {
		return err
	}

	succeededNodes := make([]string, 0)
	errMsg := ""
	for _, resp := range resps {
		if resp.err != nil {
			errMsg += fmt.Sprintf("%v: %v ", resp.host, resp.err.Error())
		}
		succeededNodes = append(succeededNodes, resp.host)
	}

	if errMsg != "" {
		// Send disjoins to nodes which succeeded joins.
		sendDisjoins(nil, succeededNodes)
		return fmt.Errorf("Failed nodes: " + errMsg)
	}

	return nil
}

// sendDisjoins sends disjoin gRPC message to provided nodes.
func sendDisjoins(cFn clusterClientFn, nodes []string) (map[string]response, error) {
	req := grpc.ClusterDisjoinReq{}
	return sendClusterGRPCs(cFn, "disjoin", nodes, &req, nil, nil)
}
