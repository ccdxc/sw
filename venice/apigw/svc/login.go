package svc

import (
	"context"
	"crypto/rand"
	"encoding/base64"
	"encoding/json"
	"errors"
	"io/ioutil"
	"net/http"
	"sync"
	"time"

	"github.com/gorilla/mux"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/manager"
	vErrors "github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	// CsrfTokenLen is length in bytes of CSRF token
	CsrfTokenLen = 32
	// LoginSvc is name under which the Login service is registered in API Gateway
	LoginSvc = "loginV1"
	// LoginSvcPath is path under which the Login service is registered in API Gateway
	LoginSvcPath = "login/"
)

var (
	// ErrInternal is returned for any server error
	ErrInternal = errors.New("internal error")
)

type loginV1GwService struct {
	defSvcProf      apigw.ServiceProfile
	svcProf         map[string]apigw.ServiceProfile
	rslvr           resolver.Interface
	tokenExpiration time.Duration
	csrfTokenLength int
	authnMgr        *manager.AuthenticationManager
}

func (s *loginV1GwService) setupSvcProfile() {
	s.defSvcProf = apigwpkg.NewServiceProfile(nil)
	s.svcProf = make(map[string]apigw.ServiceProfile)
}

func (s *loginV1GwService) GetServiceProfile(method string) (apigw.ServiceProfile, error) {
	if ret, ok := s.svcProf[method]; ok {
		return ret, nil
	}
	return nil, errors.New("not found")
}

func (s *loginV1GwService) GetCrudServiceProfile(obj string, oper apiserver.APIOperType) (apigw.ServiceProfile, error) {
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
	// cache resolver
	s.rslvr = rslvr
	apiGateway := apigwpkg.MustGetAPIGateway()
	// IP:port destination or service discovery key.
	grpcaddr := globals.APIServer
	grpcaddr = apiGateway.GetAPIServerAddr(grpcaddr)
	// create AuthenticationManager
	authnMgr, err := manager.NewAuthenticationManager(globals.APIGw, grpcaddr, s.rslvr, s.tokenExpiration)
	if err != nil {
		log.Errorf("failed to create authentication manager: %v", err)
		return ErrInternal
	}
	s.authnMgr = authnMgr
	router := mux.NewRouter()
	router.Path(login.LoginURLPath).Methods("POST").HandlerFunc(func(w http.ResponseWriter, req *http.Request) {

		body, err := ioutil.ReadAll(req.Body)
		if err != nil {
			log.Errorf("failed to read body from login request: %v", err)
			w.WriteHeader(http.StatusInternalServerError)
			vErrors.SendInternalError(w, errors.New("Failed to read body from login request: "+err.Error()))
			return
		}

		cred := &auth.PasswordCredential{}
		if err := json.Unmarshal(body, cred); err != nil {
			log.Errorf("failed to unmarshal credentials from request body: %v", err)
			w.WriteHeader(http.StatusInternalServerError)
			vErrors.SendInternalError(w, errors.New("Failed to unmarshal credentials from request body: "+err.Error()))
			return
		}

		// authenticate user
		user, err := s.login(ctx, cred)
		switch err {
		case ErrInternal:
			w.WriteHeader(http.StatusInternalServerError)
			vErrors.SendInternalError(w, err)
			return
		case nil:
			// do nothing
		default:
			w.WriteHeader(http.StatusUnauthorized)
			vErrors.SendUnauthorized(w, "Invalid username/password")
			return
		}

		// create CSRF and session token
		sessionToken, csrfToken, err := s.postLogin(ctx, user)
		if err != nil {
			log.Errorf("failed to generate CSRF token: %v", err)
			w.WriteHeader(http.StatusInternalServerError)
			vErrors.SendInternalError(w, err)
			return
		}

		w.Header().Set(apigw.GrpcMDCsrfHeader, csrfToken)
		// set cookie
		http.SetCookie(w, createCookie(sessionToken, s.tokenExpiration))
		w.WriteHeader(http.StatusOK)
		// remove user password
		user.Spec.Password = ""
		if err := json.NewEncoder(w).Encode(user); err != nil {
			log.Errorf("failed to send user json: %v", err)
			return
		}
	})
	m.Handle(login.LoginURLPath, router)
	return nil
}

func init() {
	apiGateway := apigwpkg.MustGetAPIGateway()
	svcLoginV1 := loginV1GwService{
		tokenExpiration: time.Duration(apigw.TokenExpInDays * 24 * 60 * 60), //expiration in seconds
		csrfTokenLength: CsrfTokenLen,
	}
	apiGateway.Register(LoginSvc, LoginSvcPath, &svcLoginV1)
}

func (s *loginV1GwService) login(ctx context.Context, in *auth.PasswordCredential) (*auth.User, error) {
	user, ok, err := s.authnMgr.Authenticate(in)
	if err != nil {
		log.Errorf("failed to authenticate user [%s] in tenant [%s]:  err: %v", in.Username, in.Tenant, err)
		return nil, err
	}
	if !ok {
		log.Infof("failed to authenticate user: %s in tenant %s", in.Username, in.Tenant)
		return nil, err
	}
	return user, nil
}

func (s *loginV1GwService) postLogin(ctx context.Context, in *auth.User) (string, string, error) {
	// create CSRF token
	csrfToken, err := createCSRFToken(s.csrfTokenLength)
	if err != nil {
		log.Errorf("failed to generate CSRF token: %v", err)
		return "", "", ErrInternal
	}
	claims := make(map[string]interface{})
	claims[manager.CsrfClaim] = csrfToken
	// create session token
	token, err := s.authnMgr.CreateToken(in, claims)
	if err != nil {
		log.Errorf("failed to create session token: %v", err)
		return "", "", ErrInternal
	}
	return token, csrfToken, nil
}

func createCookie(token string, expiration time.Duration) *http.Cookie {
	cookie := &http.Cookie{
		Name:     login.SessionID,
		Value:    token,
		Expires:  time.Now().Add(expiration * time.Second), //expiration in seconds
		MaxAge:   int((expiration * time.Second).Seconds()),
		Secure:   false,
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
