package auth

import (
	"testing"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
)

func getADConfig() *LdapConfig {
	return &LdapConfig{
		ServerName: "WIN-HQEM46VDNCA.pensando.io",
		TrustedCerts: `-----BEGIN CERTIFICATE-----
MIIGJDCCBQygAwIBAgITFwAAAAKQ/sQfa/q6fgAAAAAAAjANBgkqhkiG9w0BAQsF
ADBUMRIwEAYKCZImiZPyLGQBGRYCaW8xGDAWBgoJkiaJk/IsZAEZFghwZW5zYW5k
bzEkMCIGA1UEAxMbcGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBMB4XDTE4MDky
NzAxMzMxMVoXDTE5MDkyNzAxMzMxMVowJjEkMCIGA1UEAxMbV0lOLUhRRU00NlZE
TkNBLnBlbnNhbmRvLmlvMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
irNnLZWxiMlsgv0qmiZa45MpvQsogB6cqQgMhwgJ9jN8bV0zCqFmNXSmUGCInbWD
J7sjqtxu/PFlAYn0rssArdVUCYC7ZgjKF+j7iKdxH3+z0KgOyXjiJhljRDNWVHJ0
djTxYTELKThAs7d4ePbqL6l09h6Akr2jlYdzA3jlEciHk1NC9fKyvRF8Glajq7VB
ACRTtL4ewS2MvWfM/oQPZFrAqNp8q/CggTdhnd//LhZRnOqrTFZ/q0cohRDBkoGf
b/L3ysFqE4YFlOnTVbJhpOCNQ+u190GQaaSjlzO0YYtU3YA6hndFcLb14OTuX+WE
DEu81ZB4kxikba7+fi9gwwIDAQABo4IDGzCCAxcwLwYJKwYBBAGCNxQCBCIeIABE
AG8AbQBhAGkAbgBDAG8AbgB0AHIAbwBsAGwAZQByMB0GA1UdJQQWMBQGCCsGAQUF
BwMCBggrBgEFBQcDATAOBgNVHQ8BAf8EBAMCBaAweAYJKoZIhvcNAQkPBGswaTAO
BggqhkiG9w0DAgICAIAwDgYIKoZIhvcNAwQCAgCAMAsGCWCGSAFlAwQBKjALBglg
hkgBZQMEAS0wCwYJYIZIAWUDBAECMAsGCWCGSAFlAwQBBTAHBgUrDgMCBzAKBggq
hkiG9w0DBzAdBgNVHQ4EFgQUtcY/fEgmJ4jzlmHJcTLn6BCEELgwHwYDVR0jBBgw
FoAUmSdGJOIIEhdy1GZA7gyzku4HdDswgeEGA1UdHwSB2TCB1jCB06CB0KCBzYaB
ymxkYXA6Ly8vQ049cGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBLENOPVdJTi1I
UUVNNDZWRE5DQSxDTj1DRFAsQ049UHVibGljJTIwS2V5JTIwU2VydmljZXMsQ049
U2VydmljZXMsQ049Q29uZmlndXJhdGlvbixEQz1wZW5zYW5kbyxEQz1pbz9jZXJ0
aWZpY2F0ZVJldm9jYXRpb25MaXN0P2Jhc2U/b2JqZWN0Q2xhc3M9Y1JMRGlzdHJp
YnV0aW9uUG9pbnQwgc0GCCsGAQUFBwEBBIHAMIG9MIG6BggrBgEFBQcwAoaBrWxk
YXA6Ly8vQ049cGVuc2FuZG8tV0lOLUhRRU00NlZETkNBLUNBLENOPUFJQSxDTj1Q
dWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0
aW9uLERDPXBlbnNhbmRvLERDPWlvP2NBQ2VydGlmaWNhdGU/YmFzZT9vYmplY3RD
bGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MEcGA1UdEQRAMD6gHwYJKwYBBAGC
NxkBoBIEEJJg/oR1v2JMiJUKjFOlJGKCG1dJTi1IUUVNNDZWRE5DQS5wZW5zYW5k
by5pbzANBgkqhkiG9w0BAQsFAAOCAQEAAmBDGs6aH67hhmZ1jKh4kwkMMsNVVjxC
Ksig9B5YUEyOkOhAuvf41X2weZpQwwiF3BTQ96xaKrSutmVOpdyEVjuoCy9oteEb
cvjbnlpHDzk8nlwkT7oDEvVH6uNrIQ6b8hpo1rtS23JR+Fj2s+bZuk/J/WmggZYh
qzBI7uKi7rxkuDy0ejJvk7Beek/vF7qaar/bRdJQiMG/u5VmJPNCisXb9Qr8h0lI
xx3mMeRQ4B9dAtanwCIkRntp1pmTNuVXD4E/nBjIFGYY/uCDWmDuHGmG1xL26uhX
IEAsHuBf/zLR4c0QUBgs460vCgpxekUgY0XeXH9ldXqR60bxPF4UJQ==
-----END CERTIFICATE-----`,
		URL:                       "10.11.100.100:389",
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

func TestADMissingLdapAttributeMapping(t *testing.T) {
	config := getADConfig()
	testMissingLdapAttributeMapping(t, config)
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

func TestADDisabledLdapAuthenticator(t *testing.T) {
	config := getADConfig()
	testDisabledLdapAuthenticator(t, config)
}
