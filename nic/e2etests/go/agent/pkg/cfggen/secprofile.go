package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateSecurityProfiles() error {
	var cfg IOTAConfig
	var secProfiles []*netproto.SecurityProfile
	var secProfileManifest *pkg.Object
	for _, o := range c.Config.Objects {
		o := o
		if o.Kind == "SecurityProfile" {
			secProfileManifest = &o
			break
		}
	}
	if secProfileManifest == nil {
		log.Debug("Security Profile Manifest missing.")
	}

	log.Infof("Generating %v Security Profiles.", secProfileManifest.Count)

	for i := 0; i < secProfileManifest.Count; i++ {
		// Get the namespaces object
		ns, ok := c.Namespaces.Objects.([]*netproto.Namespace)
		if !ok {
			log.Errorf("Failed to cast the object %v to namespaces.", c.Namespaces.Objects)
			return fmt.Errorf("failed to cast the object %v to namespaces", c.Namespaces.Objects)
		}
		nsIdx := i % len(ns)
		namespace := ns[nsIdx]
		profileName := fmt.Sprintf("%s-%d", secProfileManifest.Name, i)
		p := netproto.SecurityProfile{
			TypeMeta: api.TypeMeta{
				Kind: "SecurityProfile",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace.Name,
				Name:      profileName,
			},
			Spec: netproto.SecurityProfileSpec{},
		}
		secProfiles = append(secProfiles, &p)
	}
	cfg.Type = "netagent"
	cfg.ObjectKey = "meta.tenant/meta.namespace/meta.name"
	cfg.RestEndpoint = "api/security/profiles/"
	cfg.Objects = secProfiles
	c.SecurityProfiles = cfg
	return nil
}
