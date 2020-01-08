package svc

import (
	"context"
	"crypto/rand"
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/gorilla/mux"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc"
	k8snet "k8s.io/apimachinery/pkg/util/net"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	// CsrfTokenLen is length in bytes of CSRF token
	CsrfTokenLen = 32
	// LoginSvc is name under which the Login service is registered in API Gateway
	LoginSvc = "loginV1"
	// LoginSvcPath is path under which the Login service is registered in API Gateway
	LoginSvcPath = "login/"
	// LoginAction for audit log
	LoginAction = "Login"
	// maxRequestBytes is the maximum bytes to read from the login request
	maxRequestBytes = 1024 * 1024
)

var (
	// ErrInternal is returned for any server error
	ErrInternal = errors.New("internal error")
	// ErrUsernameConflict is returned when local user with same name as external user exists
	ErrUsernameConflict = errors.New("local user name conflicts with external user")
)

type loginV1GwService struct {
	defSvcProf      apigw.ServiceProfile
	svcProf         map[string]apigw.ServiceProfile
	logger          log.Logger
	apiserver       string
	rslvr           resolver.Interface
	csrfTokenLength int
	authnMgr        *manager.AuthenticationManager
	permGetter      rbac.PermissionGetter
}

func (s *loginV1GwService) setupSvcProfile() {
	s.defSvcProf = apigwpkg.NewServiceProfile(nil, "", "", apiintf.UnknownOper)
	s.svcProf = make(map[string]apigw.ServiceProfile)
}

func (s *loginV1GwService) GetServiceProfile(method string) (apigw.ServiceProfile, error) {
	if ret, ok := s.svcProf[method]; ok {
		return ret, nil
	}
	return nil, errors.New("not found")
}

func (s *loginV1GwService) GetProxyServiceProfile(path string) (apigw.ServiceProfile, error) {
	return nil, nil
}

func (s *loginV1GwService) GetCrudServiceProfile(obj string, oper apiintf.APIOperType) (apigw.ServiceProfile, error) {
	name := apiserver.GetCrudServiceName(obj, oper)
	if name != "" {
		return s.GetServiceProfile(name)
	}
	return nil, errors.New("not found")
}

func (s *loginV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux,
	rslvr resolver.Interface,
	wg *sync.WaitGroup) error {
	s.logger = logger
	// cache resolver
	s.rslvr = rslvr
	apiGateway := apigwpkg.MustGetAPIGateway()
	// IP:port destination or service discovery key.
	grpcaddr := globals.APIServer
	grpcaddr = apiGateway.GetAPIServerAddr(grpcaddr)
	s.apiserver = grpcaddr
	// create AuthenticationManager
	authnMgr, err := manager.NewAuthenticationManager(globals.APIGw, grpcaddr, s.rslvr, logger)
	if err != nil {
		s.logger.Errorf("failed to create authentication manager: %v", err)
		return ErrInternal
	}
	s.authnMgr = authnMgr
	// create PermissionGetter
	s.permGetter = rbac.GetPermissionGetter(globals.APIGw, grpcaddr, s.rslvr)

	router := mux.NewRouter()
	router.Path(login.LoginURLPath).Methods("POST").HandlerFunc(func(w http.ResponseWriter, req *http.Request) {
		w.Header().Set("Content-Type", "application/json")
		body, err := ioutil.ReadAll(http.MaxBytesReader(w, req.Body, maxRequestBytes))
		if err != nil {
			s.logger.Errorf("failed to read body from login request: %v", err)
			s.httpErrorHandler(w, req, fmt.Sprintf("Failed to read body from login request: %s", err.Error()), http.StatusRequestEntityTooLarge)
			return
		}

		cred := &auth.PasswordCredential{}
		if err := json.Unmarshal(body, cred); err != nil {
			s.logger.Errorf("failed to unmarshal credentials from request body: %v", err)
			s.httpErrorHandler(w, req, fmt.Sprintf("Failed to unmarshal credentials from request body: %s", err.Error()), http.StatusBadRequest)
			return
		}

		// authenticate user
		user, err := s.login(ctx, cred)
		defer func() {
			if err != nil {
				objRef := &auth.User{}
				objRef.Defaults("all")
				objRef.Tenant = cred.Tenant
				objRef.Name = cred.Username
				recorder.Event(eventtypes.LOGIN_FAILED, fmt.Sprintf("Unsuccessful login attempt by user [%s|%s] connecting from client IPs [%v]", cred.Tenant, cred.Username, getClientIPs(req)), objRef)
			}
		}()
		switch err {
		case ErrInternal:
			s.httpErrorHandler(w, req, err.Error(), http.StatusInternalServerError)
			return
		case context.DeadlineExceeded: // thrown when API server is unreachable because cluster lost its quorum
			s.httpErrorHandler(w, req, err.Error(), http.StatusServiceUnavailable)
			return
		case nil:
			// do nothing
		default:
			s.httpErrorHandler(w, req, "Invalid username/password", http.StatusUnauthorized)
			return
		}

		// update user, create CSRF and session token
		user, sessionToken, csrfToken, exp, err := s.postLogin(ctx, user, cred.Password)
		switch err {
		case ErrUsernameConflict:
			s.httpErrorHandler(w, req, fmt.Sprintf("%s|%s %s", user.Tenant, user.Name, err.Error()), http.StatusConflict)
			return
		case context.DeadlineExceeded: // thrown when API server is unreachable because cluster lost its quorum
			s.httpErrorHandler(w, req, err.Error(), http.StatusServiceUnavailable)
			return
		case nil:
			// do nothing
		default:
			s.httpErrorHandler(w, req, err.Error(), http.StatusInternalServerError)
			return
		}
		// remove user password
		user.Spec.Password = ""
		s.audit(user, getClientIPs(req), req.RequestURI)
		w.Header().Set(apigw.GrpcMDCsrfHeader, csrfToken)
		// set cookie
		http.SetCookie(w, createCookie(sessionToken, exp))
		w.WriteHeader(http.StatusOK)

		if err := json.NewEncoder(w).Encode(user); err != nil {
			s.logger.Errorf("failed to send user json: %v", err)
			return
		}
		s.logger.Infof("user [%s|%s] successfully authenticated", user.Tenant, user.Name)
	})
	m.Handle(login.LoginURLPath, router)
	return nil
}

func init() {
	apiGateway := apigwpkg.MustGetAPIGateway()
	svcLoginV1 := loginV1GwService{
		csrfTokenLength: CsrfTokenLen,
	}
	apiGateway.Register(LoginSvc, LoginSvcPath, &svcLoginV1)
}

func (s *loginV1GwService) login(ctx context.Context, in *auth.PasswordCredential) (*auth.User, error) {
	user, ok, err := s.authnMgr.Authenticate(in)
	if err != nil {
		s.logger.Errorf("failed to authenticate user [%s|%s]:  err: %v", in.Tenant, in.Username, err)
		if strings.Contains(err.Error(), context.DeadlineExceeded.Error()) {
			err = context.DeadlineExceeded
		}
		return nil, err
	}
	if !ok {
		s.logger.Infof("failed to authenticate user: [%s|%s]", in.Tenant, in.Username)
		return nil, errors.New("authentication failed")
	}
	return user, nil
}

func (s *loginV1GwService) postLogin(ctx context.Context, in *auth.User, password string) (*auth.User, string, string, time.Time, error) {
	var exp time.Time
	// updates user status with role info. Needs to be called before creating jwt
	user, err := s.updateUserStatus(in, password)
	if err != nil {
		s.logger.Errorf("Error updating status for user [%s|%s], Err: %v", in.Tenant, in.Name, err)
		if strings.Contains(err.Error(), context.DeadlineExceeded.Error()) {
			err = context.DeadlineExceeded
		}
		return in, "", "", exp, err
	}
	// create CSRF token
	csrfToken, err := createCSRFToken(s.csrfTokenLength)
	if err != nil {
		s.logger.Errorf("failed to generate CSRF token: %v", err)
		return user, "", "", exp, ErrInternal
	}
	claims := make(map[string]interface{})
	claims[manager.CsrfClaim] = csrfToken
	// create session token
	token, exp, err := s.authnMgr.CreateToken(in, claims)
	if err != nil {
		s.logger.Errorf("failed to create session token: %v", err)
		return user, "", "", exp, ErrInternal
	}
	return user, token, csrfToken, exp, nil
}

// updateUserStatus updates user object with status(user groups, last successful login, authenticators used) in API server.
// User role information is not saved in API server
func (s *loginV1GwService) updateUserStatus(user *auth.User, password string) (*auth.User, error) {
	m, err := types.TimestampProto(time.Now())
	if err != nil {
		return nil, err
	}
	user.Status.LastLogin = &api.Timestamp{
		Timestamp: *m,
	}
	b := balancer.New(s.rslvr)
	apicl, err := apiclient.NewGrpcAPIClient(globals.APIGw, s.apiserver, s.logger, rpckit.WithBalancer(b))
	if err != nil {
		return nil, err
	}
	defer apicl.Close()
	var storedUser *auth.User
	if storedUser, err = apicl.AuthV1().User().Get(context.Background(), user.GetObjectMeta()); err != nil {
		status := apierrors.FromError(err)
		//  Create user if it is external and not found
		if user.Spec.Type == auth.UserSpec_External.String() && status.Code == http.StatusNotFound {
			user1, err := apicl.AuthV1().User().Create(context.Background(), user)
			if err != nil {
				s.logger.Errorf("Error creating external user [%s|%s], Err: %v", user.Tenant, user.Name, err)
				return nil, err
			}
			user = user1
			s.logger.Infof("External user [%s|%s] created.", user.Tenant, user.Name)
		} else {
			s.logger.Errorf("Error fetching user [%s|%s] of type [%s]): %v", user.Tenant, user.Name, user.Spec.Type, err)
			return nil, err
		}
	} else {
		// check if local user with same name as external user exists
		if storedUser.Spec.Type != user.Spec.Type {
			s.logger.Errorf("[%s] user [%s|%s] with same name as [%s] user exists",
				storedUser.Spec.Type, user.Tenant, user.Name, user.Spec.Type)
			return nil, ErrUsernameConflict
		}

		if user.Spec.Type == auth.UserSpec_Local.String() { // update local user with retries as ResourceVersion could have changed
			result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				storedUser, err = apicl.AuthV1().User().Get(ctx, user.GetObjectMeta())
				if err != nil {
					return nil, err
				}
				// set user plaintext password for local user before updating user status; user object retrieved from API server contains hashed password.
				storedUser.Spec.Password = password
				storedUser.Status = user.Status
				return apicl.AuthV1().User().Update(ctx, storedUser)
			}, 100*time.Millisecond, 20)
			if err != nil {
				return nil, err
			}
			user = result.(*auth.User)
		} else { // update external user. We are not retrying as ResourceVersion is not set by external authenticators. We always update externally authenticated user info in kvstore
			user, err = apicl.AuthV1().User().Update(context.Background(), user)
			if err != nil {
				return nil, err
			}
		}
	}

	// reset roles
	user.Status.Roles = []string{}
	roles := s.permGetter.GetRolesForUser(user)
	for _, role := range roles {
		user.Status.Roles = append(user.Status.Roles, role.Name)
	}
	// get authorized operations for user and update status
	user.Status.AccessReview = []*auth.OperationStatus{}
	user.Status.AccessReview = append(user.Status.AccessReview, authz.AuthorizedOperations(user, apigwpkg.MustGetAPIGateway().GetAuthorizer())...)
	return user, nil
}

func (s *loginV1GwService) audit(user *auth.User, clientIPs []string, reqURI string) {
	var err error
	apiGateway := apigwpkg.MustGetAPIGateway()
	auditor := apiGateway.GetAuditor()
	eventID := uuid.NewV4().String()
	creationTime, _ := types.TimestampProto(time.Now())
	addrStr := k8s.GetPodIP()
	addr, err := apiGateway.GetAddr()
	if err == nil {
		addrStr = addr.String()
	}
	event := &auditapi.AuditEvent{
		TypeMeta: api.TypeMeta{Kind: auth.Permission_AuditEvent.String()},
		ObjectMeta: api.ObjectMeta{
			Name:   eventID,
			UUID:   eventID,
			Tenant: user.Tenant,
			Labels: map[string]string{"_category": globals.Kind2Category("AuditEvent")},
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			SelfLink: fmt.Sprintf("/audit/v1/events/%s", eventID),
		},
		EventAttributes: auditapi.EventAttributes{
			Level:       auditapi.Level_Response.String(),
			Stage:       auditapi.Stage_RequestProcessing.String(),
			User:        &api.ObjectRef{Kind: string(auth.KindUser), Namespace: user.Namespace, Tenant: user.Tenant, Name: user.Name, URI: user.SelfLink},
			Resource:    &api.ObjectRef{Kind: string(auth.KindUser), Namespace: user.Namespace, Tenant: user.Tenant, Name: user.Name, URI: user.SelfLink},
			ClientIPs:   clientIPs,
			Action:      LoginAction,
			Outcome:     auditapi.Outcome_Success.String(),
			RequestURI:  reqURI,
			GatewayNode: os.Getenv("HOSTNAME"),
			GatewayIP:   addrStr,
			Data:        make(map[string]string),
		},
	}
	defer func() {
		if err != nil {
			recorder.Event(eventtypes.AUDITING_FAILED, fmt.Sprintf("Failure in recording audit event (%s) for user (%s|%s) login operation", eventID, user.Tenant, user.Name), user)
		}
	}()
	// policy checker checks whether to log audit event and populates it based on policy
	ok, _, err := audit.NewPolicyChecker().PopulateEvent(event, audit.NewResponseObjectPopulator(user, true))
	if err != nil {
		s.logger.ErrorLog("method", "audit", "msg", "error populating audit event for user login", "user", user.Name, "tenant", user.Tenant, "error", err)
		return
	}
	if ok {
		if err = auditor.ProcessEvents(event); err != nil {
			s.logger.ErrorLog("method", "audit", "msg", "error generating audit event for user login", "user", user.Name, "tenant", user.Tenant, "error", err)
			return
		}
	}
	return
}

// HTTPOtherErrorHandler converts error to api.Status before writing to response
func (s *loginV1GwService) httpErrorHandler(w http.ResponseWriter, _ *http.Request, msg string, code int) {
	status := api.Status{
		Message: []string{msg},
		Code:    int32(code),
		Result:  api.StatusResult{Str: http.StatusText(code)},
	}
	// Assume JSON encoding here.
	buf, err := json.MarshalIndent(&status, "", "  ")
	if err != nil {
		http.Error(w, msg, code)
		return
	}
	w.WriteHeader(code)
	w.Write(buf)
}

func createCookie(token string, expiration time.Time) *http.Cookie {
	cookie := &http.Cookie{
		Name:     login.SessionID,
		Value:    token,
		Expires:  expiration,
		MaxAge:   int(expiration.Sub(time.Now()).Seconds()),
		Secure:   true,
		HttpOnly: true,
		Path:     "/",
	}
	return cookie
}

func createCSRFToken(tokenLen int) (string, error) {
	tok := make([]byte, tokenLen)
	_, err := rand.Read(tok)
	if err != nil {
		return "", err
	}
	return base64.URLEncoding.EncodeToString(tok), nil
}

func getClientIPs(req *http.Request) []string {
	var clientIPs []string
	ips := k8snet.SourceIPs(req)
	for _, ip := range ips {
		clientIPs = append(clientIPs, ip.String())
	}
	if len(clientIPs) == 0 {
		// https://tools.ietf.org/html/rfc7239#section-4
		ips, ok := req.Header[apigw.Forwarded]
		if ok {
			for _, ip := range ips {
				if strings.HasPrefix(ip, "for=") {
					parsedIP := net.ParseIP(strings.TrimPrefix(ip, "for="))
					if parsedIP != nil {
						clientIPs = append(clientIPs, parsedIP.String())
					}
				}
			}
		}
	}
	return clientIPs
}
