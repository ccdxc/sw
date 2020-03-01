// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package smartnic

import (
	"crypto/x509"
	"fmt"
	"net"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/net/context"

	utils2 "github.com/pensando/sw/venice/ctrler/rollout/utils"

	"github.com/pensando/sw/api"
	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	nmdstate "github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	cmdcertutils "github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
	"github.com/pensando/sw/venice/cmd/types"
	cmdutils "github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (

	// Max retry interval in seconds for Registration retries
	// Retry interval is initially exponential and is capped
	// at 2 min.
	nicRegMaxInterval = (2 * 60)

	// Subject for a valid platform certificate
	platformCertOrg            = "Pensando Systems"
	platformCertOrgUnit        = "Pensando Manufacturing CA"
	platformSerialNumberFormat = "PID=%s SN=%s"
)

var (
	errAPIServerDown = fmt.Errorf("API Server not reachable or down")

	// HealthWatchInterval is default health watch interval
	HealthWatchInterval = 30 * time.Second

	// DeadInterval is default dead time interval, after
	// which NIC health status is declared UNKNOWN by CMD
	DeadInterval = 120 * time.Second

	// Max time to complete the entire registration sequence,
	// after which the server will cancel the request.
	// It includes the time neede to create/update the object
	// in ApiServer.
	nicRegTimeout = 30 * time.Second

	// StateMgrWarmupInterval local statemgr/cache warm up interval; once the new leader is elected,
	// this interval allows the cache to come in sync with the current system state.
	StateMgrWarmupInterval = 60 * time.Second
)

func updateCounters(nic *cluster.DistributedServiceCard, m map[string]int64) {
	switch nic.Status.AdmissionPhase {
	case cluster.DistributedServiceCardStatus_ADMITTED.String():
		m["AdmittedNICs"]++
	case cluster.DistributedServiceCardStatus_PENDING.String():
		m["PendingNICs"]++
	case cluster.DistributedServiceCardStatus_REJECTED.String():
		m["RejectedNICs"]++
	case cluster.DistributedServiceCardStatus_DECOMMISSIONED.String():
		m["DecommissionedNICs"]++
	default:
		log.Errorf("Unexpected DistributedServiceCard AdmissionPhase value: %+v", nic.Status.AdmissionPhase)
	}

	healthCond := cmdutils.GetNICCondition(nic, cluster.DSCCondition_HEALTHY)
	if healthCond != nil {
		switch healthCond.Status {
		case cluster.ConditionStatus_TRUE.String():
			m["HealthyNICs"]++
		case cluster.ConditionStatus_FALSE.String():
			m["UnhealthyNICs"]++
		case cluster.ConditionStatus_UNKNOWN.String():
			m["DisconnectedNICs"]++
		default:
			log.Errorf("Unexpected DistributedServiceCard HEALTHY condition value: %+v", healthCond)
		}
	}
}

// RPCServer implements DistributedServiceCard gRPC service.
type RPCServer struct {
	sync.Mutex

	// HealthWatchIntvl is the health watch interval
	HealthWatchIntvl time.Duration

	// DeadIntvl is the dead time interval
	DeadIntvl time.Duration

	// REST port of NMD agent
	RestPort string

	// Map of smartNICs in active retry, which are
	// marked unreachable due to failure to post naples
	// config to the NMD agent.
	RetryNicDB map[string]*cluster.DistributedServiceCard

	// reference to state manager
	stateMgr *cache.Statemgr

	versionChecker NICAdmissionVersionChecker // checks version of NIC for admission
}

// NICAdmissionVersionChecker is an interface that checks whether the nicVersion for the given SKU is allowed for admission
type NICAdmissionVersionChecker interface {
	CheckNICVersionForAdmission(nicSku string, nicVersion string) (string, string)
}

// NewRPCServer returns a DistributedServiceCard RPC server object
func NewRPCServer(healthInvl, deadInvl time.Duration, restPort string, stateMgr *cache.Statemgr, nicVersionChecker NICAdmissionVersionChecker) *RPCServer {
	return &RPCServer{
		HealthWatchIntvl: healthInvl,
		DeadIntvl:        deadInvl,
		RestPort:         restPort,
		RetryNicDB:       make(map[string]*cluster.DistributedServiceCard),
		stateMgr:         stateMgr,
		versionChecker:   nicVersionChecker,
	}
}

func (s *RPCServer) getNicKey(nic *cluster.DistributedServiceCard) string {
	return fmt.Sprintf("%s|%s", nic.Tenant, nic.Name)
}

// UpdateNicInRetryDB updates NIC entry in RetryDB
func (s *RPCServer) UpdateNicInRetryDB(nic *cluster.DistributedServiceCard) {
	s.Lock()
	defer s.Unlock()
	s.RetryNicDB[s.getNicKey(nic)] = nic
}

// NicExistsInRetryDB checks whether NIC exists in RetryDB
func (s *RPCServer) NicExistsInRetryDB(nic *cluster.DistributedServiceCard) bool {
	s.Lock()
	defer s.Unlock()
	_, exists := s.RetryNicDB[s.getNicKey(nic)]
	return exists
}

// DeleteNicFromRetryDB deletes NIC from RetryDB
func (s *RPCServer) DeleteNicFromRetryDB(nic *cluster.DistributedServiceCard) {
	s.Lock()
	defer s.Unlock()
	delete(s.RetryNicDB, s.getNicKey(nic))
}

// GetNicInRetryDB returns NIC object with match key
func (s *RPCServer) GetNicInRetryDB(key string) *cluster.DistributedServiceCard {
	s.Lock()
	defer s.Unlock()
	nicObj, ok := s.RetryNicDB[key]
	if ok {
		return nicObj
	}
	return nil
}

// validateNICPlatformCert validates the certificate provided by NAPLES in the admission request
// It only checks the certificate. It does not check possession of the private key.
func validateNICPlatformCert(cert *x509.Certificate, nic *cluster.DistributedServiceCard) error {
	if cert == nil {
		return fmt.Errorf("No certificate provided")
	}

	if nic == nil {
		return fmt.Errorf("No DistributedServiceCard object provided")
	}

	// TODO: use only Pensando PKI certs, do not accept self-signed!
	verifyOpts := x509.VerifyOptions{
		Roots: certs.NewCertPool([]*x509.Certificate{cert}),
	}

	chains, err := cert.Verify(verifyOpts)
	if err != nil || len(chains) != 1 {
		return fmt.Errorf("Certificate validation failed, err: %v", err)
	}

	if len(cert.Subject.Organization) != 1 || cert.Subject.Organization[0] != platformCertOrg {
		return fmt.Errorf("Invalid organization field in subject: %v", cert.Subject.Organization)
	}

	if len(cert.Subject.OrganizationalUnit) != 1 || cert.Subject.OrganizationalUnit[0] != platformCertOrgUnit {
		return fmt.Errorf("Invalid organizational unit field in subject: %v", cert.Subject.OrganizationalUnit)
	}

	var pid, sn string
	n, err := fmt.Sscanf(cert.Subject.SerialNumber, platformSerialNumberFormat, &pid, &sn)
	if n != 2 {
		return fmt.Errorf("Invalid PID/SN: %s", cert.Subject.SerialNumber)
	}
	if err != nil {
		return fmt.Errorf("Invalid PID/SN: %s. Error: %v", cert.Subject.SerialNumber, err)
	}

	if sn != nic.Status.SerialNum {
		return fmt.Errorf("Serial number mismatch. AdmissionRequest: %s, Certificate: %s", sn, nic.Status.SerialNum)
	}

	if cert.Subject.CommonName != nic.Status.PrimaryMAC {
		return fmt.Errorf("Name mismatch. AdmissionRequest: %s, Certificate: %s", nic.Status.PrimaryMAC, cert.Subject.CommonName)
	}

	return nil
}

// UpdateSmartNIC creates or updates the smartNIC object
func (s *RPCServer) UpdateSmartNIC(updObj *cluster.DistributedServiceCard) (*cluster.DistributedServiceCard, error) {
	var err error
	var refObj *cluster.DistributedServiceCard // reference object (before update)
	var retObj *cluster.DistributedServiceCard // return object (after update)

	// Check if object exists.
	// If it doesn't exist, do not create it, as it might have been deleted by user.
	// NIC object creation (for example during NIC registration) should always be explicit.
	nicState, err := s.stateMgr.FindSmartNIC(updObj.GetName())
	if nicState != nil && err == nil {
		nicState.Lock()
		defer nicState.Unlock()
		refObj = nicState.DistributedServiceCard
	} else {
		return nil, fmt.Errorf("Error retrieving reference object for NIC update. Err: %v, update: %+v", err, updObj)
	}

	nicName := updObj.Name

	// decide whether to send to ApiServer or not before we make any adjustment
	updateAPIServer := !runtime.FilterUpdate(refObj.Status, updObj.Status, []string{"LastTransitionTime"}, []string{"Conditions", "AdmissionPhase"})

	if updObj.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_DECOMMISSIONED.String() {
		if refObj.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_DECOMMISSIONED.String() {
			recorder.Event(eventtypes.DSC_DECOMMISSIONED, fmt.Sprintf("DSC %s(%s) decommissioned from the cluster", updObj.Spec.ID, updObj.Name), updObj)
		}
		refObj.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_DECOMMISSIONED.String()
		refObj.Status.AdmissionPhaseReason = updObj.Status.AdmissionPhaseReason
	} else {
		refHealthCond := cmdutils.GetNICCondition(refObj, cluster.DSCCondition_HEALTHY)
		updHealthCond := cmdutils.GetNICCondition(updObj, cluster.DSCCondition_HEALTHY)

		// generate event if there was a health transition
		if updHealthCond != nil && (refHealthCond == nil || refHealthCond.Status != updHealthCond.Status) {
			var evtType eventtypes.EventType
			evtType = -1
			var msg string

			switch updHealthCond.Status {
			case cluster.ConditionStatus_TRUE.String():
				evtType = eventtypes.DSC_HEALTHY
				msg = fmt.Sprintf("DSC %s(%s) is %s", updObj.Spec.ID, nicName, cluster.DSCCondition_HEALTHY.String())

			case cluster.ConditionStatus_FALSE.String():
				evtType = eventtypes.DSC_UNHEALTHY
				msg = fmt.Sprintf("DSC %s(%s) is not %s", updObj.Spec.ID, nicName, cluster.DSCCondition_HEALTHY.String())

			default:
				// this should not happen
				log.Errorf("NIC reported unknown health condition: %+v", updHealthCond)
			}

			if evtType != -1 {
				recorder.Event(evtType, msg, updObj)
				log.Infof("Generated event, type: %v, msg: %s", evtType, msg)
			}
		}

		// Ignore the time-stamp provided by NMD and replace it with our own.
		// This will help mitigating issues due to clock misalignments between Venice and NAPLES
		// As long as it gets periodic updates, CMD is happy.
		// If it happens to process an old message by mistake, the next one will correct it.
		if updHealthCond != nil {
			updHealthCond.LastTransitionTime = time.Now().UTC().Format(time.RFC3339)
		}

		// Store the update in local cache
		refObj.Status.Conditions = updObj.Status.Conditions
	}

	err = s.stateMgr.UpdateSmartNIC(refObj, updateAPIServer, false)
	if err != nil {
		log.Errorf("Error updating statemgr state for NIC %s: %v", nicName, err)
	}

	log.Debugf("UpdateSmartNIC nic: %+v", retObj)
	return refObj, err
}

func (s *RPCServer) isHostnameUnique(subj *cluster.DistributedServiceCard) bool {
	nic := s.stateMgr.GetSmartNICByID(subj.Spec.ID)
	// no need to lock nic for reading as Name is immutable
	return nic == nil || nic.Name == subj.Name
}

// RegisterNIC handles the register NIC request and upon validation creates SmartNIC object.
// NMD starts the NIC registration process when the NIC is placed in managed mode.
// NMD is expected to retry with backoff if there are API errors or if the NIC is rejected.
func (s *RPCServer) RegisterNIC(stream grpc.SmartNICRegistration_RegisterNICServer) error {
	var req *grpc.NICAdmissionRequest
	var name string

	// Canned responses in case of error
	authErrResp := &grpc.RegisterNICResponse{
		AdmissionResponse: &grpc.NICAdmissionResponse{
			Phase:  cluster.DistributedServiceCardStatus_REJECTED.String(),
			Reason: string("Authentication error"),
		},
	}

	intErrResp := &grpc.RegisterNICResponse{
		AdmissionResponse: &grpc.NICAdmissionResponse{
			Phase:  cluster.DistributedServiceCardStatus_UNKNOWN.String(),
			Reason: string("Internal error"),
		},
	}

	noClusterErrResp := &grpc.RegisterNICResponse{
		AdmissionResponse: &grpc.NICAdmissionResponse{
			Phase:  cluster.DistributedServiceCardStatus_UNKNOWN.String(),
			Reason: string("Controller node is not part of a cluster"),
		},
	}

	protoErrResp := &grpc.RegisterNICResponse{
		AdmissionResponse: &grpc.NICAdmissionResponse{
			Phase:  cluster.DistributedServiceCardStatus_UNKNOWN.String(),
			Reason: string("Internal error"),
		},
	}

	// There is no way to specify timeouts for individual Send/Recv calls in Golang gRPC,
	// so we execute the entire registration sequence under a single timeout.
	// https://github.com/grpc/grpc-go/issues/445
	// https://github.com/grpc/grpc-go/issues/1229
	procRequest := func(ctx context.Context) (interface{}, error) {

		msg, err := stream.Recv()
		if err != nil || ctx.Err() != nil {
			return nil, errors.Wrapf(err, "Error receiving admission request")
		}

		if msg.AdmissionRequest == nil {
			return protoErrResp, fmt.Errorf("Protocol error: no AdmissionRequest in message")
		}

		req = msg.AdmissionRequest
		naplesNIC := req.GetNic()
		name = naplesNIC.Name

		nicCerts := req.GetCerts()
		if len(nicCerts) == 0 {
			return authErrResp, errors.Wrapf(err, "No SmartNIC certificate found")
		}
		cert, err := x509.ParseCertificate(nicCerts[0])
		if err != nil {
			return authErrResp, errors.Wrapf(err, "Invalid SmartNIC certificate")
		}

		// Validate the factory cert obtained in the request
		err = validateNICPlatformCert(cert, &naplesNIC)
		if err != nil {
			return authErrResp, errors.Wrapf(err, "Invalid certificate, name: %v, cert subject: %v", naplesNIC.Name, cert.Subject)
		}

		// check the CSR
		csr, err := x509.ParseCertificateRequest(req.GetClusterCertSignRequest())
		if err != nil {
			return protoErrResp, errors.Wrapf(err, "Invalid certificate request")
		}
		err = csr.CheckSignature()
		if err != nil {
			return protoErrResp, errors.Wrapf(err, "Certificate request has invalid signature")
		}

		// Send challenge and cluster trust chain
		challenge, err := certs.GeneratePoPNonce()
		if err != nil {
			return intErrResp, errors.Wrapf(err, "Error generating challenge")
		}

		trustChain := [][]byte{}
		for _, c := range env.CertMgr.Ca().TrustChain() {
			trustChain = append(trustChain, c.Raw)
		}

		authReq := &grpc.RegisterNICResponse{
			AuthenticationRequest: &grpc.AuthenticationRequest{
				Challenge:  challenge,
				TrustChain: trustChain,
			},
		}

		err = stream.Send(authReq)
		if err != nil || ctx.Err() != nil {
			return nil, errors.Wrapf(err, "Error sending auth request")
		}

		// Receive challenge response
		msg, err = stream.Recv()
		if err != nil || ctx.Err() != nil {
			return nil, errors.Wrapf(err, "Error receiving auth response")
		}

		authResp := msg.AuthenticationResponse
		if authResp == nil {
			return protoErrResp, fmt.Errorf("Protocol error: no AuthenticationResponse in msg")
		}

		err = certs.VerifyPoPChallenge(challenge, authResp.ClaimantRandom, authResp.ChallengeResponse, cert)
		if err != nil {
			return authErrResp, errors.Wrapf(err, "Authentication failed")
		}

		// NAPLES is genuine, sign the CSR
		clusterCert, err := env.CertMgr.Ca().Sign(csr)
		if err != nil {
			return intErrResp, errors.Wrap(err, "Error signing certificate request")
		}

		// NAPLES must be in network-managed mode
		if strings.ToLower(naplesNIC.Spec.MgmtMode) != strings.ToLower(nmd.MgmtMode_NETWORK.String()) {
			return intErrResp, fmt.Errorf("Cannot admit SmartNIC because it is not in network-managed mode [%v]", naplesNIC.Spec.MgmtMode)
		}

		log.Infof("Validated NIC: %s", name)

		// If we make it to this point, we know the NAPLES is authentic.
		// However, there are still reasons why the admission may fail.
		// We create or update the object in ApiServer so that user knows
		// if something went wrong and can take action.

		// Get the Cluster object
		clusterObj, err := s.stateMgr.GetCluster()
		if err != nil {
			return intErrResp, errors.Wrapf(err, "Error getting Cluster object")
		}

		var nicObj *cluster.DistributedServiceCard
		var smartNICObjExists bool // does the SmartNIC object already exist ?
		nicObjState, err := s.stateMgr.FindSmartNIC(name)
		if err != nil {
			if err != memdb.ErrObjectNotFound {
				return intErrResp, errors.Wrapf(err, "Error reading NIC object")
			}
			// NIC object is not present. Create the NIC object based on the template provided by NAPLES and update it accordingly.
			smartNICObjExists = false
			nicObj = &naplesNIC
		} else {
			nicObjState.Lock()
			defer nicObjState.Unlock()
			// we need to work on a copy of the cached object, as opposed to making modifications in-place, because
			// UpdateSmartNIC has logic that triggers only if the updated object differs from the cached one
			updNIC := cluster.DistributedServiceCard{}
			_, err = nicObjState.DistributedServiceCard.Clone(&updNIC)
			nicObj = &updNIC
			smartNICObjExists = true
		}

		// If SmartNIC object does not exist, we initialize Spec.Admit using the value of cluster.Spec.AutoAdmitDSCs
		// If it exists, we honor the value that is currently there
		// Note that even if Spec.Admit = true, the SmartNIC can end up in rejected state if it fails subsequent checks.
		if !smartNICObjExists {
			nicObj.Spec.Admit = clusterObj.Spec.AutoAdmitDSCs
			nicObj.Spec.DSCProfile = globals.DefaultDSCProfile

		}

		// If hostname supplied by NAPLES is not unique, reject but still create SmartNIC object
		if !s.isHostnameUnique(&naplesNIC) {
			nicObj.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_REJECTED.String()
			nicObj.Status.AdmissionPhaseReason = "Hostname is not unique"
			nicObj.Status.Conditions = nil
		} else {
			// Re-initialize status as it might have changed
			nicObj.Status = naplesNIC.Status
			// clear out host pairing so that it will be recomputed
			nicObj.Status.Host = ""
			// clear out conditions as we will only accept them from health updates
			nicObj.Status.Conditions = nil

			// mark as admitted or pending based on current value of Spec.Admit
			if nicObj.Spec.Admit == true {
				nicObj.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_ADMITTED.String()
				nicObj.Status.AdmissionPhaseReason = ""
				// If the NIC is admitted, override all spec parameters with the new supplied values,
				// except those that are owned by Venice.
				nicObj.Spec = naplesNIC.Spec
				nicObj.Spec.Admit = true
			} else {
				nicObj.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_PENDING.String()
				nicObj.Status.AdmissionPhaseReason = "SmartNIC waiting for manual admission"
			}
		}

		// If NIC was decommissioned from Venice, override previous Spec.MgmtMode
		nicObj.Spec.MgmtMode = cluster.DistributedServiceCardSpec_NETWORK.String()
		nicObj.Status.VersionMismatch = false

		log.Infof("NIC %s with SKU %s and version %s AdmissionPhase %v", name, nicObj.Status.GetDSCSku(), nicObj.Status.GetDSCVersion(), nicObj.Status.AdmissionPhase)
		status, veniceVersion := s.versionChecker.CheckNICVersionForAdmission(nicObj.Status.GetDSCSku(), nicObj.Status.GetDSCVersion())
		if status == utils2.RequestRolloutNaples && nicObj.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() {
			log.Infof("ForceRollout: NIC %s with SKU %s and version %s is requested to rollout to version %s ", name, nicObj.Status.GetDSCSku(), nicObj.Status.GetDSCVersion(), veniceVersion)
			nicObj.Status.VersionMismatch = true
			nicObj.Status.AdmissionPhaseReason = "Incompatible NIC Version. Force Rollout Requested."
		}

		// Create or update SmartNIC object in ApiServer
		if smartNICObjExists {
			err = s.stateMgr.UpdateSmartNIC(nicObj, true, true)
		} else {
			var sns *cache.SmartNICState
			sns, err = s.stateMgr.CreateSmartNIC(nicObj, true)
			sns.Lock()
			defer sns.Unlock()
		}
		if err != nil {
			status := apierrors.FromError(err)
			log.Errorf("Error creating or updating smartNIC object. Create:%v, obj:%+v err:%v status:%v", !smartNICObjExists, nicObj, err, status)
			return intErrResp, errors.Wrapf(err, "Error updating smartNIC object")
		}

		if nicObj.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_REJECTED.String() {
			recorder.Event(eventtypes.DSC_REJECTED,
				fmt.Sprintf("Admission for DSC %s(%s) was rejected, reason: %s", nicObj.Spec.ID, nicObj.Name, nicObj.Status.AdmissionPhaseReason), nicObj)

			return &grpc.RegisterNICResponse{
				AdmissionResponse: &grpc.NICAdmissionResponse{
					Phase:  cluster.DistributedServiceCardStatus_REJECTED.String(),
					Reason: nicObj.Status.AdmissionPhaseReason,
				},
			}, nil
		}

		// ADMITTED or PENDING
		okResp := &grpc.RegisterNICResponse{
			AdmissionResponse: &grpc.NICAdmissionResponse{
				Phase:       nicObj.Status.AdmissionPhase,
				Controllers: env.QuorumNodes,
			},
		}

		if nicObj.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() {
			okResp.AdmissionResponse.ClusterCert = &certapi.CertificateSignResp{
				Certificate: &certapi.Certificate{
					Certificate: clusterCert.Raw,
				},
			}
			okResp.AdmissionResponse.CaTrustChain = cmdcertutils.GetCaTrustChain(env.CertMgr)
			okResp.AdmissionResponse.TrustRoots = cmdcertutils.GetTrustRoots(env.CertMgr)
			if nicObj.Status.VersionMismatch {
				okResp.AdmissionResponse.Reason = "Incompatible NIC Version. Force Rollout Requested."
				okResp.AdmissionResponse.RolloutVersion = veniceVersion
			}
		}
		return okResp, nil
	}

	// if we are not part of a cluster yet we cannot process admission requests
	clusterObj, err := s.stateMgr.GetCluster()
	if err != nil || clusterObj == nil {
		retErr := fmt.Errorf("Rejecting RegisterNIC request from %s, cluster not formed yet, err: %v", ctxutils.GetPeerAddress(stream.Context()), err)
		log.Errorf("%v", retErr)
		stream.Send(noClusterErrResp)
		return retErr
	}

	ctx, cancel := context.WithTimeout(stream.Context(), nicRegTimeout)
	defer cancel()
	resp, err := utils.ExecuteWithContext(ctx, procRequest)

	// if we have a response we send it back to the client, otherwise
	// it means we timed out and can just terminate the request
	if resp != nil {
		regNICResp := resp.(*grpc.RegisterNICResponse)
		stream.Send(regNICResp)
		if strings.ToLower(regNICResp.AdmissionResponse.Phase) == strings.ToLower(cluster.DistributedServiceCardStatus_ADMITTED.String()) {
			log.Infof("SmartNIC %s is admitted to the cluster", name)
		}
	}

	if err != nil {
		log.Errorf("Error processing NIC admission request, name: %v, err: %v", name, err)
	}

	return err
}

// UpdateNIC is the handler for the UpdateNIC() RPC invoked by NMD
func (s *RPCServer) UpdateNIC(ctx context.Context, req *grpc.UpdateNICRequest) (*grpc.UpdateNICResponse, error) {
	// Update smartNIC object with CAS semantics
	obj := req.GetNic()
	nicObj, err := s.UpdateSmartNIC(&obj)

	if err != nil || nicObj == nil {
		log.Errorf("Error updating SmartNIC object: %+v err: %v", obj, err)
		return &grpc.UpdateNICResponse{}, err
	}

	return &grpc.UpdateNICResponse{}, nil // no need to send back the full update
}

//ListSmartNICs lists all smartNICs matching object selector
func (s *RPCServer) ListSmartNICs(ctx context.Context, sel *api.ObjectMeta) ([]*cluster.DistributedServiceCard, error) {
	var niclist []*cluster.DistributedServiceCard
	// get all smartnics
	nics, err := s.stateMgr.ListSmartNICs()
	if err != nil {
		return nil, err
	}

	// walk all smartnics and add it to the list
	for _, nic := range nics {
		if sel.GetName() == nic.GetName() {
			niclist = append(niclist, nic.DistributedServiceCard)
		}
	}

	return niclist, nil
}

// WatchNICs watches smartNICs objects for changes and sends them as streaming rpc
func (s *RPCServer) WatchNICs(sel *api.ObjectMeta, stream grpc.SmartNICUpdates_WatchNICsServer) error {
	// watch for changes
	watcher := memdb.Watcher{Name: "dsc-cmd"}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	defer close(watcher.Channel)
	s.stateMgr.WatchObjects("DistributedServiceCard", &watcher)
	defer s.stateMgr.StopWatchObjects("DistributedServiceCard", &watcher)

	// first get a list of all smartnics
	nics, err := s.ListSmartNICs(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of smartnics. Err: %v", err)
		return err
	}

	ctx := stream.Context()

	// send the objects out as a stream
	for _, nic := range nics {
		watchEvt := grpc.SmartNICEvent{
			EventType: api.EventType_CreateEvent,
			Nic:       *nic,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	// loop forever on watch channel
	log.Infof("WatchNICs entering watch loop for SmartNIC: %s", sel.GetName())
	var sentEvent grpc.SmartNICEvent
	for {
		select {
		// read from channel
		case evt, ok := <-watcher.Channel:
			if !ok {
				log.Errorf("Error reading from channel. Closing watch")
				return errors.New("Error reading from channel")
			}

			// convert to smartnic object
			nic, err := cache.SmartNICStateFromObj(evt.Obj)
			if err != nil {
				return err
			}

			if sel.GetName() != nic.GetName() {
				continue
			}

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			}

			nic.Lock()
			// construct the smartnic event object
			watchEvt := grpc.SmartNICEvent{
				EventType: etype,
				Nic:       *nic.DistributedServiceCard,
			}
			nic.Unlock()
			send := !runtime.FilterUpdate(sentEvent, watchEvt, []string{"LastTransitionTime"}, nil)
			if send {
				log.Infof("Sending watch event for SmartNIC: %+v", watchEvt)
				err = stream.Send(&watchEvt)
				if err != nil {
					log.Errorf("Error sending stream. Err: %v closing watch", err)
					return err
				}
				sentEvent = watchEvt
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// MonitorHealth periodically inspects that health status of
// smartNIC objects every 30sec. For NICs that haven't received
// health updates in over 120secs, CMD would mark the status as unknown.
func (s *RPCServer) MonitorHealth() {

	log.Info("Launching Monitor Health")
	for {
		select {

		case <-time.After(s.HealthWatchIntvl):
			if env.LeaderService == nil || !env.LeaderService.IsLeader() {
				// only leader gets updates from the NIC and so can detect when a NIC becomes unresponsive
				continue
			}

			// if the leader transitioned in past 60s, then wait for the cache to be warmed up
			if time.Since(env.LeaderService.LastTransitionTime()) <= StateMgrWarmupInterval {
				time.Sleep(StateMgrWarmupInterval)
				s.stateMgr.MarkSmartNICsDirty()
				continue
			}

			// Get a list of all existing smartNICs
			// Take it from the cache, not ApiServer, because we do not propagate state
			// back to ApiServer unless there is a real change.
			// condition.LastTransitionTime on ApiServer actually represents the last time
			// there was a transition, not the last time the condition was reported.
			// In the cache instead we set LastTransitionTime for each update.
			// Also compute new cluster-level SmartNIC metrics

			updCounters := types.GetSmartNICMetricsZeroMap()
			nicStates, err := s.stateMgr.ListSmartNICs()
			if err != nil {
				log.Errorf("Failed to getting a list of nics, err: %v", err)
				continue
			}

			log.Infof("Health watch timer callback, #nics: %d ", len(nicStates))

			// Iterate on smartNIC objects
			for _, nicState := range nicStates {
				nicState.Lock()
				nic := nicState.DistributedServiceCard
				if nic.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() {
					for i := 0; i < len(nic.Status.Conditions); i++ {
						condition := nic.Status.Conditions[i]
						// Inspect HEALTH condition with status that is marked healthy or unhealthy (i.e not unknown)
						if condition.Type == cluster.DSCCondition_HEALTHY.String() && condition.Status != cluster.ConditionStatus_UNKNOWN.String() {
							// parse the last reported time
							t, err := time.Parse(time.RFC3339, condition.LastTransitionTime)
							if err != nil {
								log.Errorf("Failed parsing last transition time for NIC health, nic: %+v, err: %v", nic, err)
								break
							}
							// if the time elapsed since last health update is over
							// the deadInterval, update the Health status to unknown
							if err == nil && time.Since(t) > s.DeadIntvl {
								// update the nic health status to unknown
								lastUpdateTime := nic.Status.Conditions[i].LastTransitionTime
								log.Infof("Updating NIC health to unknown, nic: %+v", nic)
								nic.Status.Conditions[i].Status = cluster.ConditionStatus_UNKNOWN.String()
								nic.Status.Conditions[i].LastTransitionTime = time.Now().UTC().Format(time.RFC3339)
								nic.Status.Conditions[i].Reason = fmt.Sprintf("NIC health update not received since %s", lastUpdateTime)
								// push the update back to ApiServer
								err := s.stateMgr.UpdateSmartNIC(nic, true, false)
								if err != nil {
									log.Errorf("Failed updating the NIC health status to unknown, nic: %s err: %s", nic.Name, err)
								}
								recorder.Event(eventtypes.DSC_UNREACHABLE,
									fmt.Sprintf("DSC %s(%s) is %s", nic.Spec.ID, nic.Name, eventtypes.DSC_UNREACHABLE.String()), nicState.DistributedServiceCard)
							}
							break
						}
					}
				}
				updateCounters(nic, updCounters)
				nicState.Unlock()
			}
			if env.MetricsService != nil {
				env.MetricsService.UpdateCounters(updCounters)
				log.Infof("Updated SmartNIC metrics, values: %+v", updCounters)
			} else {
				log.Errorf("Error updating SmartNIC metrics, service not available. Values: %+v", updCounters)
			}
		}
	}
}

// InitiateNICRegistration does the naples config POST for managed mode
// to the NMD REST endpoint using the configured Mgmt-IP. Failures will
// be retried for maxIters and after that NIC status will be updated to
// UNREACHABLE.
// Further retries for UNREACHABLE nics will be handled by
// NIC health watcher which runs periodically.
func (s *RPCServer) InitiateNICRegistration(nic *cluster.DistributedServiceCard) {

	var retryInterval time.Duration
	retryInterval = 1

	var resp nmdstate.NaplesConfigResp

	// check if Nic exists in RetryDB
	if s.NicExistsInRetryDB(nic) == true {
		s.UpdateNicInRetryDB(nic)
		log.Debugf("Nic registration retry is ongoing, nic: %s", nic.Name)
		return
	}

	// Add Nic to the RetryDB
	s.UpdateNicInRetryDB(nic)
	log.Infof("Initiating nic registration for Naples, MAC: %s IP:%+v", nic.Name, nic.Spec.IPConfig.IPAddress)

	for {
		select {
		case <-time.After(retryInterval * time.Second):

			if s.NicExistsInRetryDB(nic) == false {
				// If NIC is deleted stop the retry
				return
			}

			nicObj := s.GetNicInRetryDB(s.getNicKey(nic))
			controller, _, err := net.SplitHostPort(env.SmartNICRegRPCServer.GetListenURL())
			if err != nil {
				log.Errorf("Error parsing unauth RPC server URL %s: %v", env.SmartNICRegRPCServer.GetListenURL(), err)
				// retry
				continue
			}

			// Config to switch to Managed mode
			naplesCfg := nmd.DistributedServiceCard{
				ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
				TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
				Spec: nmd.DistributedServiceCardSpec{
					Mode:        nmd.MgmtMode_NETWORK.String(),
					NetworkMode: cluster.DistributedServiceCardSpec_NetworkModes_name[cluster.DistributedServiceCardSpec_NetworkModes_vvalue[nicObj.Spec.NetworkMode]],
					PrimaryMAC:  nicObj.Name,
					Controllers: []string{controller},
					ID:          nicObj.Spec.ID,
					IPConfig:    nicObj.Spec.IPConfig,
				},
			}

			ip, _, _ := net.ParseCIDR(nicObj.Spec.IPConfig.IPAddress)
			nmdURL := fmt.Sprintf("http://%s:%s%s", ip, s.RestPort, nmdstate.ConfigURL)
			log.Infof("Posting Naples config: %+v to Naples-Ip: %s", naplesCfg, nmdURL)

			err = netutils.HTTPPost(nmdURL, &naplesCfg, &resp)
			if err == nil {
				log.Infof("Nic registration post request to Naples node successful, nic:%s", nic.Name)
				s.DeleteNicFromRetryDB(nic)
				return
			}

			// Update NIC status condition.
			// Naples may be unreachable if the configured Mgmt-IP is either invalid
			// or if it is valid but part of another Venice cluster and in that case
			// the REST port would have been shutdown (hence unreachable) after it is
			// admitted in managed mode.
			log.Errorf("Retrying, failed to post naples config, nic: %s err: %+v resp: %+v", nic.Name, err, resp)
			nic := cluster.DistributedServiceCard{
				TypeMeta:   nicObj.TypeMeta,
				ObjectMeta: nicObj.ObjectMeta,
				Status: cluster.DistributedServiceCardStatus{
					Conditions: []cluster.DSCCondition{
						{
							Type:               cluster.DSCCondition_NIC_HEALTH_UNKNOWN.String(),
							Status:             cluster.ConditionStatus_TRUE.String(),
							LastTransitionTime: time.Now().UTC().Format(time.RFC3339),
							Reason:             fmt.Sprintf("Failed to post naples config after several attempts, response: %+v", resp),
							Message:            fmt.Sprintf("Naples REST endpoint: %s:%s is not reachable", nic.Spec.IPConfig.IPAddress, s.RestPort),
						},
					},
				},
			}
			_, err = s.UpdateSmartNIC(&nic)
			if err != nil {
				log.Errorf("Error updating the NIC status as unreachable nic:%s err:%v", nicObj.Name, err)
			}

			// Retry with backoff, capped at nicRegMaxInterval
			if 2*retryInterval <= nicRegMaxInterval {
				retryInterval = 2 * retryInterval
			} else {
				retryInterval = nicRegMaxInterval
			}
		}
	}
}
