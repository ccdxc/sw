package utils

import (
	"io/ioutil"
	"math"
	"os"
	"strings"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	pdsGRPCDefaultPort    = "11358" //PDS_GRPC_PORT_UPGMGR
	pdsGRPCDefaultBaseURL = "localhost"
	//PdsUpgStatusFail Upgrade Failure
	PdsUpgStatusFail = 2
	//PdsUpgStatusSuccess Upgrade Success
	PdsUpgStatusSuccess = 0
	//PdsUpgStatusInProcess Upgrade InProgress
	PdsUpgStatusInProcess = 1
	pdsUpgStatusFile      = "/update/pds_upg_status.txt"
)

// CreateUPGMGRNewGRPCClient creates a grpc connection to UPGMGR
func CreateUPGMGRNewGRPCClient() (*grpc.ClientConn, error) {

	srvURL := pdsGRPCDefaultBaseURL + ":" + pdsGRPCDefaultPort
	var grpcOpts []grpc.DialOption
	grpcOpts = append(grpcOpts, grpc.WithMaxMsgSize(math.MaxInt32-1))
	grpcOpts = append(grpcOpts, grpc.WithInsecure())
	rpcClient, err := grpc.Dial(srvURL, grpcOpts...)

	if err != nil {
		log.Errorf("Creating gRPC Client failed. Server URL: %s", srvURL)
		return nil, err
	}

	return rpcClient, err
}

//ProcessPdsUpgStatus get the upgrade status from /update/pds_upg_status.txt for Apollo
func ProcessPdsUpgStatus() int {

	if _, err := os.Stat(pdsUpgStatusFile); os.IsNotExist(err) {
		log.Infof("/update/pds_upg_status.txt not found %v", err)
		return PdsUpgStatusFail
	}

	data, err := ioutil.ReadFile(pdsUpgStatusFile)
	if err != nil {
		log.Infof(" unable to read /update/pds_upg_status.txt %v", err)
		return PdsUpgStatusFail
	}
	log.Infof("file content %v", data)
	strList := strings.Split(string(data), ":")

	if strList[len(strList)-1] == "success" {
		return PdsUpgStatusSuccess
	}
	if strList[len(strList)-1] == "failed" {
		return PdsUpgStatusFail
	}
	return PdsUpgStatusInProcess
}
