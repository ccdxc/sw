package auth

import (
	"testing"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
)

func getADConfig() *LdapConfig {
	tinfo.ldapAddr = "ldap.test.pensando.io:389"
	return &LdapConfig{
		ServerName: "WIN-2NGFKBD39CL.pensando.io",
		TrustedCerts: `-----BEGIN CERTIFICATE-----
MIIGJDCCBQygAwIBAgITIgAAAALXObNR3SxqpgAAAAAAAjANBgkqhkiG9w0BAQsF
ADBUMRIwEAYKCZImiZPyLGQBGRYCaW8xGDAWBgoJkiaJk/IsZAEZFghwZW5zYW5k
bzEkMCIGA1UEAxMbcGVuc2FuZG8tV0lOLTJOR0ZLQkQzOUNMLUNBMB4XDTIwMDMw
NzA3MjQxN1oXDTIxMDMwNzA3MjQxN1owJjEkMCIGA1UEAxMbV0lOLTJOR0ZLQkQz
OUNMLnBlbnNhbmRvLmlvMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
3EVXOpzxCGqEWNqkEAv5c2+if+WAyIOm3X85M4HCIycetMnbNr0iMk+FEJYANiH8
TPe0YBgVnrwWlZlA+ae+741e4kHsf6C0atQEsDgDbDKTZBfk/Sitl/X99zXXf/qN
FPEtirHL3mwodvzgJN1Sk2X+FCqGKynfcN2q8dsfw2SnL8XTw0RUdBRrQiQyhMcx
qDfBFpU6B9TvNCSpIHHZ3ma8vmuZoP1LCMGiEkktTFHaW9z+XSIlWOjAiGDu9IcD
izOGF76XjmNHTkoozhj/RcM8k8kh8euMpoklhmrzLeMbeIg068lLAgbW76zMp8is
5d2tGgHW1OahI9VDtyV5qQIDAQABo4IDGzCCAxcwLwYJKwYBBAGCNxQCBCIeIABE
AG8AbQBhAGkAbgBDAG8AbgB0AHIAbwBsAGwAZQByMB0GA1UdJQQWMBQGCCsGAQUF
BwMCBggrBgEFBQcDATAOBgNVHQ8BAf8EBAMCBaAweAYJKoZIhvcNAQkPBGswaTAO
BggqhkiG9w0DAgICAIAwDgYIKoZIhvcNAwQCAgCAMAsGCWCGSAFlAwQBKjALBglg
hkgBZQMEAS0wCwYJYIZIAWUDBAECMAsGCWCGSAFlAwQBBTAHBgUrDgMCBzAKBggq
hkiG9w0DBzAdBgNVHQ4EFgQUBYz486PyLoX+6Zf7Q0zpwNlYnp8wHwYDVR0jBBgw
FoAUNkg6ZiKW43MHVV4tDkxblnc26GYwgeEGA1UdHwSB2TCB1jCB06CB0KCBzYaB
ymxkYXA6Ly8vQ049cGVuc2FuZG8tV0lOLTJOR0ZLQkQzOUNMLUNBLENOPVdJTi0y
TkdGS0JEMzlDTCxDTj1DRFAsQ049UHVibGljJTIwS2V5JTIwU2VydmljZXMsQ049
U2VydmljZXMsQ049Q29uZmlndXJhdGlvbixEQz1wZW5zYW5kbyxEQz1pbz9jZXJ0
aWZpY2F0ZVJldm9jYXRpb25MaXN0P2Jhc2U/b2JqZWN0Q2xhc3M9Y1JMRGlzdHJp
YnV0aW9uUG9pbnQwgc0GCCsGAQUFBwEBBIHAMIG9MIG6BggrBgEFBQcwAoaBrWxk
YXA6Ly8vQ049cGVuc2FuZG8tV0lOLTJOR0ZLQkQzOUNMLUNBLENOPUFJQSxDTj1Q
dWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0
aW9uLERDPXBlbnNhbmRvLERDPWlvP2NBQ2VydGlmaWNhdGU/YmFzZT9vYmplY3RD
bGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MEcGA1UdEQRAMD6gHwYJKwYBBAGC
NxkBoBIEEHplBgWxeXlEhwck2HST1UmCG1dJTi0yTkdGS0JEMzlDTC5wZW5zYW5k
by5pbzANBgkqhkiG9w0BAQsFAAOCAQEASSFATmfn2F52fUls3uHhErlvh7t11VZO
AoGsia2bNDDEwDS48VlYLfhGD5WD7BaF0TYoLvnVwb6yFULLVr3Kzek8hDHSb8jD
/KBw09MmWI1iuwhaqNlbknlrpJ4NJWE6Ooa/FnDPQu66s21AwAT0TYJBzB1CX0l3
dHday05cOTS7S5C8TyEjDhsKU2vA9pkIQ+oZsUdObkQx5EJqMQqUTRO2DO892hTs
ESVRcLzPcJt+CGQoeaLfI9OsFOjDkWIRE3ucaC+z6LS44pLa+FwlnG6PCUJDDsUr
IYV2F9mCcxWpNNlVFA2zFrNmblqPlZq8t43eZxwxU+pbW0t+PoroIw==
-----END CERTIFICATE-----`,
		URL:                       tinfo.ldapAddr,
		BaseDN:                    "DC=pensando,DC=io",
		BindDN:                    "pensando\\Administrator",
		BindPassword:              "G0pensando",
		UserAttribute:             "sAMAccountName",
		UserObjectClassAttribute:  "organizationalPerson",
		GroupAttribute:            "memberOf",
		GroupObjectClassAttribute: "group",
		TenantAttribute:           "department",
		ReferralServer:            "openldap",
		LdapServer:                "openldapref",
		LdapUser:                  "testuser",
		LdapUserGroupsDN:          []string{"CN=Domain Users,CN=Users,DC=pensando,DC=io", "CN=Users,CN=Builtin,DC=pensando,DC=io"},
		LdapUserPassword:          "G0pensando",
	}
}

func TestADAuthenticate(t *testing.T) {
	config := getADConfig()
	testAuthenticate(t, config)
}

func TestADIncorrectPasswordAuthentication(t *testing.T) {
	config := getADConfig()
	testIncorrectPasswordAuthentication(t, config)
}

func TestADIncorrectUserAuthentication(t *testing.T) {
	config := getADConfig()
	testIncorrectUserAuthentication(t, config)
}

func TestADIncorrectLdapAttributeMapping(t *testing.T) {
	config := getADConfig()
	testIncorrectLdapAttributeMapping(t, config)
}

func TestADIncorrectBaseDN(t *testing.T) {
	config := getADConfig()
	testIncorrectBaseDN(t, config)
}

func TestADIncorrectBindPassword(t *testing.T) {
	config := getADConfig()
	testIncorrectBindPassword(t, config)
}
