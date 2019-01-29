package cfggen

import (
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

func (c *CfgGen) GenerateSecurityProfiles() error {
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
		nsIdx := i % len(c.Namespaces)
		namespace := c.Namespaces[nsIdx]
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
	c.SecurityProfiles = secProfiles
	return nil
}
