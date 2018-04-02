package manager

import (
	"encoding/base64"
	"encoding/json"
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"

	"gopkg.in/square/go-jose.v2"
	"gopkg.in/square/go-jose.v2/jwt"

	"sort"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var testRoles = []string{"NetworkAdminRole", "SecurityAdminRole"}

var testUserObj = auth.User{
	TypeMeta: api.TypeMeta{Kind: "User"},
	ObjectMeta: api.ObjectMeta{
		Tenant: tenant,
		Name:   testUser,
	},
	Spec: auth.UserSpec{
		Fullname: "Test User",
		Password: testPassword,
		Email:    "testuser@pensandio.io",
		Type:     auth.UserSpec_LOCAL.String(),
	},
	Status: auth.UserStatus{
		Roles: testRoles,
	},
}

var testCsrfToken = "csrftoken"

// createToken creates JWT token
//   alg: JWT signing algorithm for example HS256
//   secret: JWT signing secret
//   expiration: JWT expiration claim
//   issuer: JWT issuer claim
func createToken(alg jose.SignatureAlgorithm, secret []byte, expiration time.Duration, issuer string) string {
	sig, err := jose.NewSigner(jose.SigningKey{Algorithm: alg, Key: secret}, (&jose.SignerOptions{}).WithType("JWT"))
	if err != nil {
		panic(fmt.Sprintf("Unable to create a signer: Err: %v", err))

	}
	claims := jwt.Claims{
		Subject:  testUser,
		Issuer:   issuer,
		Expiry:   jwt.NewNumericDate(time.Now().Add(expiration)),
		IssuedAt: jwt.NewNumericDate(time.Now()),
	}
	// venice custom claims
	customClaims := make(map[string]interface{})
	customClaims[TenantClaim] = tenant
	customClaims[CsrfClaim] = testCsrfToken
	customClaims[RolesClaim] = testRoles

	token, err := jwt.Signed(sig).Claims(claims).Claims(customClaims).CompactSerialize()
	if err != nil {
		panic(fmt.Sprintf("Unable to create JWT token: Err: %v", err))
	}
	return token
}

func createHeadlessToken(alg jose.SignatureAlgorithm, secret []byte, expiration time.Duration, issuer string) string {
	token := createToken(alg, secret, expiration, issuer)
	parts := strings.Split(token, ".")
	return fmt.Sprintf("%s.%s", parts[1], parts[2])
}

func createJwtMgr() *jwtMgr {
	// pre-create JWT header
	header, err := createJWTHeader()
	if err != nil {
		panic(fmt.Sprintf("Error creating JWT header: Err: %v", err))
	}
	// pre-create signer to sign tokens
	signer, err := jose.NewSigner(jose.SigningKey{Algorithm: signatureAlgorithm, Key: secret}, (&jose.SignerOptions{}).WithType(headerTypeValue))
	if err != nil {
		panic(fmt.Sprintf("Error creating signer: Err: %v", err))
	}
	return &jwtMgr{
		signer:     signer,
		secret:     secret,
		expiration: time.Duration(expiration),
		header:     header,
	}
}

func TestCreateToken(t *testing.T) {
	jwtMgr := createJwtMgr()
	tokStr, err := jwtMgr.createJWTToken(&testUserObj, nil)

	AssertOk(t, err, "Error creating JWT token")

	token, err := jwt.ParseSigned(tokStr)
	AssertOk(t, err, fmt.Sprintf("Error parsing JWT token: %s", tokStr))

	standardClaims := jwt.Claims{}
	customClaims := make(map[string]interface{})
	err = token.Claims(secret, &standardClaims, &customClaims)
	AssertOk(t, err, fmt.Sprintf("Error verifying JWT token: %s", tokStr))

	Assert(t, standardClaims.Subject == testUserObj.Name, "Incorrect subject claim")
	Assert(t, customClaims[TenantClaim] == testUserObj.Tenant, "Incorrect tenant claim")
	Assert(t, rolesEqual(customClaims[RolesClaim].([]interface{}), testRoles),
		fmt.Sprintf("Incorrect roles, expected {%+v}, got {%+v}", testRoles, customClaims[RolesClaim]))
}

func TestCreateTokenWithCsrf(t *testing.T) {
	jwtMgr := createJwtMgr()
	privateClaims := make(map[string]interface{})
	privateClaims[CsrfClaim] = testCsrfToken
	tokStr, err := jwtMgr.createJWTToken(&testUserObj, privateClaims)

	AssertOk(t, err, "Error creating JWT token")

	token, err := jwt.ParseSigned(tokStr)
	AssertOk(t, err, fmt.Sprintf("Error parsing JWT token: %s", tokStr))

	standardClaims := jwt.Claims{}
	customClaims := make(map[string]interface{})
	err = token.Claims(secret, &standardClaims, &customClaims)
	AssertOk(t, err, fmt.Sprintf("Error verifying JWT token: %s", tokStr))

	Assert(t, standardClaims.Subject == testUserObj.Name, "Incorrect subject claim")
	Assert(t, customClaims[TenantClaim] == testUserObj.Tenant, "Incorrect tenant claim")
	Assert(t, rolesEqual(customClaims[RolesClaim].([]interface{}), testRoles),
		fmt.Sprintf("Incorrect roles, expected {%+v}, got {%+v}", testRoles, customClaims[RolesClaim]))
	Assert(t, customClaims[CsrfClaim] == testCsrfToken, fmt.Sprintf("Incorrect csrf token returned: %s", customClaims[CsrfClaim]))
}

func TestCreateTokenWithMissingUserInfo(t *testing.T) {
	jwtMgr := createJwtMgr()
	tokStr, err := jwtMgr.createJWTToken(nil, nil)
	Assert(t, tokStr == "", fmt.Sprintf("Token returned for missing user: %s", tokStr))
	Assert(t, err != nil, "No error returned for missing user")
	Assert(t, err == ErrMissingUserInfo, "Incorrect error returned for missing user info")
}

func TestValidateToken(t *testing.T) {
	jwtMgr := createJwtMgr()
	tokInfo, ok, err := jwtMgr.ValidateToken(testHS512JWTToken)
	Assert(t, ok, "Token validation failed")
	AssertOk(t, err, "Error validating token")
	Assert(t, tokInfo[SubClaim] == testUser, "Incorrect user name")
	Assert(t, tokInfo[TenantClaim] == tenant, "Incorrect tenant name")
	Assert(t, rolesEqual(tokInfo[RolesClaim].([]interface{}), testRoles),
		fmt.Sprintf("Incorrect roles, expected {%+v}, got {%+v}", testRoles, tokInfo[RolesClaim]))
	Assert(t, tokInfo[CsrfClaim] == testCsrfToken, fmt.Sprintf("Incorrect csrf token returned: %s", tokInfo[CsrfClaim]))
}

func TestValidateTokenWithInvalidSecret(t *testing.T) {
	// create token with an invalid secret
	token := createHeadlessToken(signatureAlgorithm, []byte("invalid secret"), time.Hour, issuerClaimValue)
	jwtMgr := createJwtMgr()
	tokInfo, ok, err := jwtMgr.ValidateToken(token)
	Assert(t, !ok, "Token validation should fail for invalid secret")
	Assert(t, err != nil, "No error returned for invalid secret")
	Assert(t, tokInfo == nil, "Token info returned while validating invalid token")
}

func TestValidateTokenWithExpiredToken(t *testing.T) {
	// create expired token
	token := createHeadlessToken(signatureAlgorithm, secret, -time.Hour, issuerClaimValue)
	jwtMgr := createJwtMgr()
	tokInfo, ok, err := jwtMgr.ValidateToken(token)
	Assert(t, !ok, "Token validation should fail for expired token")
	Assert(t, err != nil, "No error returned for expired token")
	Assert(t, err == jwt.ErrExpired, "Incorrect error returned for expired token")
	Assert(t, tokInfo == nil, "Token info returned while validating expired token")
}

func TestValidateTokenWithInvalidIssuer(t *testing.T) {
	// create token with invalid issuer
	token := createHeadlessToken(signatureAlgorithm, secret, time.Hour, "invalid issuer")
	jwtMgr := createJwtMgr()
	tokInfo, ok, err := jwtMgr.ValidateToken(token)
	Assert(t, !ok, "Token validation should fail for token with invalid issuer")
	Assert(t, err != nil, "No error returned for token with invalid issuer")
	Assert(t, err == jwt.ErrInvalidIssuer, "Incorrect error returned for token with invalid issuer")
	Assert(t, tokInfo == nil, "Token info returned while validating token with invalid issuer")
}

// modifySubjectClaimInToken returns headless jwt token with modified payload and original signature
func modifySubjectClaimInToken(token string) string {
	parts := strings.Split(token, ".")
	claimBytes, err := base64.RawURLEncoding.DecodeString(parts[0])
	if err != nil {
		panic("unable to decode payload to modify user claim")
	}
	claimMap := make(map[string]interface{})
	if err := json.Unmarshal(claimBytes, &claimMap); err != nil {
		panic("unable to unmarshal claim bytes")
	}
	// modify subject claim
	claimMap["sub"] = "invalidUser"
	claimBytes, err = json.Marshal(claimMap)
	if err != nil {
		panic("unable to marshal modified claims")
	}
	return fmt.Sprintf("%s.%s", base64.RawURLEncoding.EncodeToString(claimBytes), parts[1])
}

func TestValidateTokenWithModifiedClaims(t *testing.T) {
	correctToken := createHeadlessToken(signatureAlgorithm, secret, time.Duration(expiration)*time.Second, issuerClaimValue)
	modifiedToken := modifySubjectClaimInToken(correctToken)
	jwtMgr := createJwtMgr()
	_, ok, err := jwtMgr.ValidateToken(correctToken)
	Assert(t, ok, "Token validation failed for valid token")
	AssertOk(t, err, "Error returned for valid token")

	// validating token with modified "sub" claim while keeping the signature same
	tokInfo, ok, err := jwtMgr.ValidateToken(modifiedToken)
	Assert(t, !ok, "Token validation should fail for invalid signature")
	Assert(t, err != nil, "No error returned for invalid signature contained in token")
	Assert(t, err == jose.ErrCryptoFailure, "Expecting jose.ErrCryptoFailure error")
	Assert(t, tokInfo == nil, "Token info returned while validating token with invalid signature")
}

func TestValidateTokenWithNoneAlg(t *testing.T) {
	// token with "none" alg
	token := "eyJhbGciOiJub25lIiwidHlwIjoiSldUIn0.eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19."
	jwtMgr := &jwtMgr{
		secret:     secret,
		expiration: time.Duration(expiration),
	}
	_, privateClaims, ok, err := jwtMgr.validateJWTToken(token)
	Assert(t, !ok, "Token validation should fail for token with 'none' alg type")
	Assert(t, err != nil, "No error returned for 'none' alg token")
	Assert(t, err == ErrInvalidSignature, "Incorrect error returned for 'none' alg token")
	Assert(t, privateClaims == nil, "Claims returned while validating 'none' alg token")
}

func BenchmarkValidateToken(b *testing.B) {
	jwtMgr := createJwtMgr()
	b.ResetTimer()
	b.Run("ValidateToken", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			jwtMgr.ValidateToken(testHS512JWTToken)
		}
	})
}

func TestCreateHeadlessJWTToken(t *testing.T) {
	jwtMgr, err := NewJWTManager(secret, time.Duration(expiration))
	AssertOk(t, err, "Error creating JWT token manager")
	headlessToken, err := jwtMgr.CreateToken(&testUserObj, nil)
	AssertOk(t, err, "Error creating headless JWT token")
	Assert(t, len(strings.Split(headlessToken, ".")) == 2, "Headless token should contain only two parts")
	headerStr, err := createJWTHeader()
	AssertOk(t, err, "Error creating JWT header")

	jwtToken := fmt.Sprintf("%s.%s", headerStr, headlessToken)
	token, err := jwt.ParseSigned(jwtToken)
	AssertOk(t, err, fmt.Sprintf("Error parsing JWT token: %s", jwtToken))

	standardClaims := jwt.Claims{}
	customClaims := make(map[string]interface{})
	err = token.Claims(secret, &standardClaims, &customClaims)
	AssertOk(t, err, fmt.Sprintf("Error verifying JWT token: %s", jwtToken))

	Assert(t, standardClaims.Subject == testUserObj.Name, "Incorrect subject claim")
	Assert(t, customClaims[TenantClaim] == testUserObj.Tenant, "Incorrect tenant claim")
	Assert(t, rolesEqual(customClaims[RolesClaim].([]interface{}), testRoles),
		fmt.Sprintf("Incorrect roles, expected {%+v}, got {%+v}", testRoles, customClaims[RolesClaim]))
}

func TestCreateJWTHeader(t *testing.T) {
	headerStr, err := createJWTHeader()
	AssertOk(t, err, "Error creating JWT header")
	// base64 decode the headers
	rawProtected, err := base64.RawURLEncoding.DecodeString(headerStr)
	AssertOk(t, err, "Error base64 decoding JWT header")
	// check if "alg" header is HS512
	headers := &jwtHeader{}
	err = json.Unmarshal(rawProtected, &headers)
	AssertOk(t, err, "Unable to unmarshal JWT headers")
	Assert(t, jose.SignatureAlgorithm(headers.Algorithm) == signatureAlgorithm, "Incorrect algorithm specified in JWT header")
	// check if "typ" header is JWT
	Assert(t, headers.Type == headerTypeValue, "Incorrect type specified in JWT header")
}

func erroneousTokenData() map[string]string {
	tokens := make(map[string]string)

	tokens["none alg without header"] = "eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19."
	tokens["without header and random signature bits"] = "eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19.asdfwerds"
	tokens["none alg with header"] = "eyJhbGciOiJub25lIiwidHlwIjoiSldUIn0.eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19"
	tokens["HS256 alg with header and missing signature"] = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19"
	tokens["HS256 alg without header but with . separator for header"] = ".eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19.pKNdTo12gtsg719b6AV50T2hFDB3wV-hYNhRV76ZMA4"
	tokens["No header and signature but with . separator for header and signature"] = ".eyJleHAiOjE1MTY3MzUzMDQsImlzcyI6InZlbmljZSIsInN1YiI6InRlc3QiLCJ1c2VyIjp7IktpbmQiOiJVc2VyIiwibWV0YSI6eyJDcmVhdGlvblRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk1vZFRpbWUiOiIxOTcwLTAxLTAxVDAwOjAwOjAwWiIsIk5hbWUiOiJ0ZXN0IiwiVGVuYW50IjoiZGVmYXVsdCJ9LCJzcGVjIjp7ImVtYWlsIjoidGVzdHVzZXJAcGVuc2FuZGlvLmlvIiwiZnVsbG5hbWUiOiJUZXN0IFVzZXIiLCJwYXNzd29yZCI6InBlbnNhbmRvbzAiLCJ0eXBlIjoiTE9DQUwifSwic3RhdHVzIjp7InJvbGVzIjpbIk5ldHdvcmtBZG1pblJvbGUiLCJTZWN1cml0eUFkbWluUm9sZSJdfX19."

	return tokens
}

func TestErroneousTokens(t *testing.T) {
	jwtMgr, err := NewJWTManager(secret, time.Duration(expiration))
	AssertOk(t, err, "Error creating JWT token manager")
	for testtype, token := range erroneousTokenData() {
		tokenInfo, ok, err := jwtMgr.ValidateToken(token)

		Assert(t, !ok, fmt.Sprintf("[%v] Token validation should fail", testtype))
		Assert(t, err != nil, fmt.Sprintf("[%v] No error returned for invalid token", testtype))
		Assert(t, err != jwt.ErrExpired, fmt.Sprintf("[%v] It should error out before token expiration check", testtype))
		Assert(t, tokenInfo == nil, fmt.Sprintf("[%v] Token info returned for invalid token", testtype))
	}
}

func TestRemoveJWTHeader(t *testing.T) {
	token := createToken(signatureAlgorithm, secret, time.Duration(expiration)*time.Second, issuerClaimValue)
	headless := removeJWTHeader(token)
	parts := strings.Split(token, ".")
	Assert(t, headless == fmt.Sprintf("%s.%s", parts[1], parts[2]), "Invalid headless token")
}

func TestGet(t *testing.T) {
	jwtMgr, err := NewJWTManager(secret, time.Duration(expiration))
	AssertOk(t, err, "Error creating JWT token manager")
	tests := []struct {
		claim              string
		exists             bool
		expectedClaimValue interface{}
	}{
		{SubClaim, true, testUser},
		{IssuerClaim, true, issuerClaimValue},
		{IssuedAtClaim, true, "non nil"},
		{ExpClaim, true, "non nil"},
		{NotBeforeClaim, false, nil},
		{AudienceClaim, false, nil},
		{IDClaim, false, nil},
		{CsrfClaim, true, testCsrfToken},
	}
	for _, test := range tests {
		val, ok, err := jwtMgr.Get(testHS512JWTToken, test.claim)
		Assert(t, ok == test.exists, fmt.Sprintf("Get returned unexpected value for [%s] claim", val))
		AssertOk(t, err, fmt.Sprintf("Error fetching [%s] claim", val))
		switch test.claim {
		case "iat", "exp":
			Assert(t, val != nil, fmt.Sprintf("got nil value for claim [%s]", test.claim))
		default:
			Assert(t, val == test.expectedClaimValue, fmt.Sprintf("Incorrect claim value, expected [%s], got [%s]", test.expectedClaimValue, val))
		}
	}
	// test with expired token
	// create expired token
	token := createHeadlessToken(signatureAlgorithm, secret, -time.Hour, issuerClaimValue)
	val, ok, err := jwtMgr.Get(token, "sub")
	Assert(t, !ok, "Get should fail for expired token")
	Assert(t, err != nil, "No error returned for expired token")
	Assert(t, err == jwt.ErrExpired, "Incorrect error returned for expired token")
	Assert(t, val == nil, "claim value returned for expired token")
}

func rolesEqual(roleClaims []interface{}, roles []string) bool {
	sort.Slice(roleClaims, func(i, j int) bool {
		return roleClaims[i].(string) < roleClaims[j].(string)
	})
	sort.Slice(roles, func(i, j int) bool {
		return roles[i] < roles[j]
	})

	strRoleClaims := make([]string, len(roleClaims))
	for i, claim := range roleClaims {
		strRoleClaims[i] = fmt.Sprint(claim)
	}
	return reflect.DeepEqual(strRoleClaims, roles)
}
