package dependencies

import (
	"fmt"
	"strings"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// meta will allow us to hold only the dependent object's metas
type meta struct {
	T api.TypeMeta
	O api.ObjectMeta
}

// StateDependencies maintains goroutine safe references for dependent objects.
// Keyed by Kind|Tenant|Namespace|Name
type StateDependencies struct {
	sync.Mutex
	DB map[string]map[string]bool
}

// NewDepSolver returns a new dependency solver instance
func NewDepSolver() *StateDependencies {
	var solver StateDependencies
	solver.DB = make(map[string]map[string]bool)
	return &solver
}

// Add will create a parent to child relationships
func (s *StateDependencies) Add(parent interface{}, children ...interface{}) error {
	s.Lock()
	defer s.Unlock()

	tMeta, oMeta, err := s.resolveObjectType(parent)
	if err != nil {
		log.Errorf("could not resolve parent type from %v", parent)
		return err
	}

	parentMeta := meta{
		T: tMeta,
		O: oMeta,
	}
	parentKey, _, err := s.composeKeySelfLink(&parentMeta)
	if err != nil {
		log.Errorf("could not generate parent key from %v. Err: %v", parentMeta, err)
		return err
	}
	// check if the child map is created
	_, ok := s.DB[parentKey]
	if !ok {
		s.DB[parentKey] = make(map[string]bool)
	}

	for _, c := range children {
		tMeta, oMeta, err := s.resolveObjectType(c)
		if err != nil {
			log.Errorf("could not resolve type from %v. Err: %v", c, err)
			return err
		}
		m := meta{
			T: tMeta,
			O: oMeta,
		}
		childKey, _, err := s.composeKeySelfLink(&m)
		if err != nil {
			log.Errorf("could not generate child key from %v. Err: %v", m, err)
			return err
		}
		s.DB[parentKey][childKey] = true
	}
	return nil
}

// Remove will delete the specified parent to child relationships
func (s *StateDependencies) Remove(parent interface{}, children ...interface{}) error {
	s.Lock()
	defer s.Unlock()
	tMeta, oMeta, err := s.resolveObjectType(parent)
	if err != nil {
		log.Errorf("could not resolve parent type from %v", parent)
		return err
	}

	parentMeta := meta{
		T: tMeta,
		O: oMeta,
	}
	parentKey, _, err := s.composeKeySelfLink(&parentMeta)
	if err != nil {
		log.Errorf("could not generate parent key from %v. Err: %v", parentMeta, err)
		return err
	}

	for _, c := range children {
		tMeta, oMeta, err := s.resolveObjectType(c)
		if err != nil {
			log.Errorf("could not resolve type from %v. Err: %v", c, err)
			return err
		}
		m := meta{
			T: tMeta,
			O: oMeta,
		}
		childKey, _, err := s.composeKeySelfLink(&m)
		if err != nil {
			log.Errorf("could not generate child key from %v. Err: %v", m, err)
			return err
		}
		delete(s.DB[parentKey], childKey) // = append(s.DB[parentKey], m)
	}
	return nil
}

// Solve checks if a given object has pending dependencies.
func (s *StateDependencies) Solve(o interface{}) error {
	s.Lock()
	defer s.Unlock()
	tMeta, oMeta, err := s.resolveObjectType(o)
	if err != nil {
		log.Errorf("could not generate the key from %v. Err: %v", o, err)
		return err
	}
	m := meta{
		T: tMeta,
		O: oMeta,
	}
	key, _, err := s.composeKeySelfLink(&m)
	if err != nil {
		log.Errorf("could not generate the self link from %v. Err: %v", o, err)
		return err
	}

	deps, ok := s.DB[key]
	if !ok || len(deps) == 0 {
		// No dependencies, safe for deletion
		return nil
	}
	// return ErrCantDelete from deps
	return s.composeErrCantDelete(deps)
}

// resolveObjectType will resolve the interface to a concrete type and return its self link and lookup key
func (s *StateDependencies) resolveObjectType(o interface{}) (api.TypeMeta, api.ObjectMeta, error) {
	switch o.(type) {
	case *netproto.Tenant:
		tn := o.(*netproto.Tenant)
		return tn.TypeMeta, tn.ObjectMeta, nil
	case *netproto.Namespace:
		ns := o.(*netproto.Namespace)
		return ns.TypeMeta, ns.ObjectMeta, nil
	case *netproto.Network:
		nw := o.(*netproto.Network)
		return nw.TypeMeta, nw.ObjectMeta, nil
	case *netproto.Endpoint:
		ep := o.(*netproto.Endpoint)
		return ep.TypeMeta, ep.ObjectMeta, nil
	case *netproto.Route:
		rt := o.(*netproto.Route)
		return rt.TypeMeta, rt.ObjectMeta, nil
	case *netproto.NatPool:
		np := o.(*netproto.NatPool)
		return np.TypeMeta, np.ObjectMeta, nil
	case *netproto.NatBinding:
		nb := o.(*netproto.NatBinding)
		return nb.TypeMeta, nb.ObjectMeta, nil
	case *netproto.NatPolicy:
		np := o.(*netproto.NatPolicy)
		return np.TypeMeta, np.ObjectMeta, nil
	case *netproto.IPSecSAEncrypt:
		ie := o.(*netproto.IPSecSAEncrypt)
		return ie.TypeMeta, ie.ObjectMeta, nil
	case *netproto.IPSecSADecrypt:
		id := o.(*netproto.IPSecSADecrypt)
		return id.TypeMeta, id.ObjectMeta, nil
	case *netproto.IPSecPolicy:
		ip := o.(*netproto.IPSecPolicy)
		return ip.TypeMeta, ip.ObjectMeta, nil
	case *netproto.SGPolicy:
		sp := o.(*netproto.SGPolicy)
		return sp.TypeMeta, sp.ObjectMeta, nil
	case *netproto.Tunnel:
		tu := o.(*netproto.Tunnel)
		return tu.TypeMeta, tu.ObjectMeta, nil
	case *netproto.SecurityGroup:
		sg := o.(*netproto.SecurityGroup)
		return sg.TypeMeta, sg.ObjectMeta, nil
	case *netproto.TCPProxyPolicy:
		tcp := o.(*netproto.TCPProxyPolicy)
		return tcp.TypeMeta, tcp.ObjectMeta, nil
	case *netproto.Port:
		port := o.(*netproto.Port)
		return port.TypeMeta, port.ObjectMeta, nil
	case *netproto.SecurityProfile:
		profile := o.(*netproto.SecurityProfile)
		return profile.TypeMeta, profile.ObjectMeta, nil
	default:
		log.Errorf("Invalid object type %v", o)
		err := fmt.Errorf("invalid object type. %v", o)
		return api.TypeMeta{}, api.ObjectMeta{}, err
	}
}

// ObjectKey returns object key from object meta
func (s *StateDependencies) ObjectKey(meta api.ObjectMeta, T api.TypeMeta) string {
	if len(T.Kind) == 0 {
		return ""
	}
	switch strings.ToLower(T.Kind) {
	case "tenant":
		return fmt.Sprintf("tenant|%s", meta.Name)
	case "namespace":
		return fmt.Sprintf("namespace|%s|%s", meta.Tenant, meta.Name)
	default:
		return fmt.Sprintf("%s|%s|%s|%s", strings.ToLower(T.Kind), meta.Tenant, meta.Namespace, meta.Name)
	}
}

// composeErrCantDelete returns an ErrCannotDelete with the self links of dependent objects
func (s *StateDependencies) composeErrCantDelete(deps map[string]bool) *types.ErrCannotDelete {
	var delErr types.ErrCannotDelete
	for d := range deps {
		m := s.composeMetaFromKey(d)
		_, selfLink, _ := s.composeKeySelfLink(m)
		delErr.References = append(delErr.References, selfLink)
	}
	return &delErr
}

func (s *StateDependencies) composeKeySelfLink(m *meta) (key, selfLink string, err error) {
	key = s.ObjectKey(m.O, m.T)
	switch strings.ToLower(m.T.Kind) {
	case "tenant":
		selfLink = fmt.Sprintf("/api/tenants/%v", m.O.Name)
		return
	case "namespace":
		selfLink = fmt.Sprintf("/api/namespaces/%v/%v", m.O.Tenant, m.O.Name)
		return
	case "network":
		selfLink = fmt.Sprintf("/api/networks/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "endpoint":
		selfLink = fmt.Sprintf("/api/endpoints/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "route":
		selfLink = fmt.Sprintf("/api/routes/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "natpool":
		selfLink = fmt.Sprintf("/api/nat/pools/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "natbinding":
		selfLink = fmt.Sprintf("/api/nat/bindings/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "natpolicy":
		selfLink = fmt.Sprintf("/api/nat/policies/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "ipsecsaencrypt":
		selfLink = fmt.Sprintf("/api/ipsec/encryption/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "ipsecsadecrypt":
		selfLink = fmt.Sprintf("/api/ipsec/decryption/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "ipsecpolicy":
		selfLink = fmt.Sprintf("/api/ipsec/policies/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "sgpolicy":
		selfLink = fmt.Sprintf("/api/security/policies/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "tunnel":
		selfLink = fmt.Sprintf("/api/tunnels/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "securitygroup":
		selfLink = fmt.Sprintf("/api/sgs/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "tcpproxypolicy":
		selfLink = fmt.Sprintf("/api/tcp/proxies/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "port":
		selfLink = fmt.Sprintf("/api/system/ports/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	case "securityprofile":
		selfLink = fmt.Sprintf("/api/security/profiles/%v/%v/%v", m.O.Tenant, m.O.Namespace, m.O.Name)
		return
	default:
		log.Errorf("Invalid object type %v. Obj: %v", m.T, m.O)
		err = fmt.Errorf("invalid object type. %v. Obj: %v", m.T, m.O)
		return
	}
}

func (s *StateDependencies) composeMetaFromKey(k string) *meta {
	components := strings.Split(k, "|")

	switch len(components) {
	case 2:
		// case for tenant
		m := meta{
			T: api.TypeMeta{Kind: components[0]},
			O: api.ObjectMeta{Name: components[1]},
		}
		return &m
	case 3:
		// case for namespace
		m := meta{
			T: api.TypeMeta{Kind: components[0]},
			O: api.ObjectMeta{Tenant: components[1],
				Name: components[2]},
		}
		return &m
	case 4:
		// case for all other objects
		m := meta{
			T: api.TypeMeta{Kind: components[0]},
			O: api.ObjectMeta{Tenant: components[1],
				Namespace: components[2],
				Name:      components[3]},
		}
		return &m
	default:
		return nil
	}

}
