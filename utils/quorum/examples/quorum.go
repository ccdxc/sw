package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/go-martini/martini"
	"github.com/pensando/sw/utils/log"

	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/quorum/store"
)

var quorumIntf quorum.Interface

const (
	membersURL = "/members"
)

func main() {
	var id, name, dataDir, cfgFile, peerPort, clientPort, members, memberIPs string
	var existing bool
	flag.StringVar(&id, "id", "testCluster", "identifier for the cluster/member")
	flag.StringVar(&name, "name", "", "name of this member")
	flag.StringVar(&dataDir, "data-dir", "/var/lib/etcd", "directory to store kvstore data")
	flag.StringVar(&cfgFile, "cfg-file", "/etc/etcd.conf", "file to store kvstore configuration")
	flag.StringVar(&peerPort, "peer-port", "5001", "peer port")
	flag.StringVar(&clientPort, "client-port", "5002", "client port")
	flag.StringVar(&members, "members", "", "comma separated list of member names")
	flag.StringVar(&memberIPs, "member-ips", "", "comma separated list of member ips")
	flag.BoolVar(&existing, "existing", false, "existing or new cluster")
	flag.Parse()

	if len(name) == 0 {
		log.Fatalf("name cannot be empty")
	}

	membersSlice := strings.Split(members, ",")
	memberIPsSlice := strings.Split(memberIPs, ",")
	if len(membersSlice) == 0 || len(memberIPsSlice) == 0 || len(membersSlice) != len(memberIPsSlice) {
		log.Fatalf("Invalid members or memberIPs, must be non zero and of same length: %v, %v", members, memberIPs)
	}

	memberIdx := -1
	for ii, member := range membersSlice {
		if member == name {
			memberIdx = ii
			break
		}
	}

	if memberIdx == -1 {
		log.Fatalf("This member %v not found in member list %v", name, members)
	}

	memberCfgs := make([]quorum.Member, 0)
	for ii := 0; ii < len(membersSlice); ii++ {
		memberCfgs = append(memberCfgs, quorum.Member{
			Name:       membersSlice[ii],
			PeerURLs:   []string{fmt.Sprintf("http://%s:%s", memberIPsSlice[ii], peerPort)},
			ClientURLs: []string{fmt.Sprintf("http://%s:%s", memberIPsSlice[ii], clientPort)},
		})
	}

	config := &quorum.Config{
		Type:       store.KVStoreTypeEtcd,
		ID:         id,
		DataDir:    dataDir,
		CfgFile:    cfgFile,
		MemberName: name,
		Existing:   existing,
		Members:    memberCfgs,
	}

	var err error

	quorumIntf, err = store.New(config)
	if err != nil {
		log.Fatalf("Failed to start quorum with error: %v", err)
	}

	log.Infof("Created quorum successfully")

	resp, err := quorumIntf.List()
	if err != nil {
		log.Fatalf("Failed to list quorum members with error: %v", err)
	}

	log.Infof("Found members: %v", resp)

	mux := newHTTPServer()
	port := ":9001"

	log.Infof("Starting http server at %v", port)
	mux.RunOnAddr(port)
}

// newHTTPServer creates a http server for member endpoints.
func newHTTPServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Post(membersURL, MemberAddHandler)
	m.Delete(membersURL+"/:id", MemberRemoveHandler)
	m.Get(membersURL, MemberListHandler)

	return m
}

// MemberAddHandler adds a member to the quorum.
func MemberAddHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	member := quorum.Member{}
	if err := decoder.Decode(&member); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	if err := quorumIntf.Add(&member); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	return http.StatusOK, fmt.Sprintf("Member %v added to quorum", member)
}

// MemberRemoveHandler deletes a member.
func MemberRemoveHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	id := params["id"]

	if id == "" {
		return http.StatusBadRequest, fmt.Sprintf("Member id is not specified")

	}

	memberID, err := strconv.ParseUint(id, 10, 64)
	if err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Member id %v is invalid, needs to be uint64", id)
	}

	if err := quorumIntf.Remove(memberID); err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Member %q removal failed: %v\n", id, err)
	}

	return http.StatusOK, fmt.Sprintf("Member %q removed\n", id)
}

// MemberListHandler lists all members.
func MemberListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	members, err := quorumIntf.List()

	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Members list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&members)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}
