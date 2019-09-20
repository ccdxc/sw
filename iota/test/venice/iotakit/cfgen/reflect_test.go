package cfgen

import (
	"encoding/json"
	"fmt"
	"os"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
)

func TestNetworkSecurityPolicyGen(t *testing.T) {
	sgp := &security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "sgpolicy-{{iter}}",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
		},
	}

	ofile, err := os.OpenFile("/tmp/refjson.data", os.O_RDWR|os.O_CREATE, 0755)
	if err != nil {
		panic(err)
	}

	sgpIterCtx := newIterContext()
	for ii := 0; ii < 10; ii++ {
		fromIPAddressTmpl := fmt.Sprintf("{{ipv4:%d.x.x.x}}", ii+10)
		toIPAddressTmpl := fmt.Sprintf("{{ipv4:%d.x.x.x}}", ii+100)
		rule := security.SGRule{
			Action:          "{{rand:PERMIT|DENY|REJECT}}",
			FromIPAddresses: []string{fromIPAddressTmpl},
			ToIPAddresses:   []string{toIPAddressTmpl},
			ProtoPorts:      []security.ProtoPort{{Protocol: "{{rand:tcp|udp}}", Ports: "{{iter:5000-50000}}"}},
		}

		tSgp := sgpIterCtx.transform(sgp).(*security.NetworkSecurityPolicy)

		rules := []security.SGRule{}
		ruleIterCtx := newIterContext()
		for ii := 0; ii < 15; ii++ {
			tRule := ruleIterCtx.transform(rule)
			rules = append(rules, tRule.(security.SGRule))
		}
		tSgp.Spec.Rules = rules
		j, err := json.MarshalIndent(tSgp, "", "  ")
		if err != nil {
			panic(err)
		}
		ofile.Write(j)
	}
}
