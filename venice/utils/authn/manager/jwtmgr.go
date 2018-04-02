package manager

import (
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"time"

	"gopkg.in/square/go-jose.v2"
	"gopkg.in/square/go-jose.v2/jwt"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	issuerClaimValue = "venice" //TODO: Should this be cluster identification?

	/* HS256 Block(B) = 64bytes,  HashOutput(L)=32bytes, 32bytes <= KeySize <= 64bytes
	   HS512 Block(B) = 128bytes, HashOutput(L)=64bytes, 64bytes <= KeySize <= 128bytes
	   HMAC => H(K XOR opad, H(K XOR ipad, text))
	     where H = hash function, K = key, ipad = the byte 0x36 repeated B times, opad = the byte 0x5C repeated B times
	*/
	signatureAlgorithm   = jose.HS512
	headerAlgorithmValue = "HS512"
	headerTypeValue      = "JWT"
)

var (
	// ErrInvalidSignature error is returned when token signature contains incorrect algorithm or multiple signatures are present
	ErrInvalidSignature = errors.New("invalid signature")
	// ErrInvalidTokenFormat error is returned when token doesn't consist of two parts
	ErrInvalidTokenFormat = errors.New("invalid token format")
)

type jwtMgr struct {
	signer     jose.Signer
	secret     []byte
	expiration time.Duration
	header     string
}

type jwtHeader struct {
	Algorithm string `json:"alg"`
	Type      string `json:"typ"`
}

// NewJWTManager creates new instance of JWT TokenManager
//  secret: secret used to sign JWT. It should be at least 64 bytes(512 bits) for HS512. We will use 128 bytes generated
//          using a secure random number generator.
//  expiration: expiration time in seconds for which token is valid
func NewJWTManager(secret []byte, expiration time.Duration) (TokenManager, error) {
	// pre-create JWT header
	header, err := createJWTHeader()
	if err != nil {
		return nil, err
	}
	// pre-create signer to sign tokens
	signer, err := jose.NewSigner(jose.SigningKey{Algorithm: signatureAlgorithm, Key: secret}, (&jose.SignerOptions{}).WithType(headerTypeValue))
	if err != nil {
		log.Errorf("Unable to create a signer: Err: %v", err)
		return nil, err
	}
	return &jwtMgr{
		signer:     signer,
		secret:     secret,
		expiration: expiration,
		header:     header,
	}, nil
}

// CreateToken creates headless JWT token without header
func (t *jwtMgr) CreateToken(user *auth.User, privateClaims map[string]interface{}) (string, error) {
	token, err := t.createJWTToken(user, privateClaims)
	if err != nil {
		return "", err
	}
	return removeJWTHeader(token), nil
}

// createJWTToken creates JWT token with header
func (t *jwtMgr) createJWTToken(user *auth.User, privateClaims map[string]interface{}) (string, error) {
	if user == nil || user.Name == "" {
		log.Errorf("User information is required to create a JWT token")
		return "", ErrMissingUserInfo
	}
	// standard jwt claims like sub, iss, exp
	claims := jwt.Claims{
		Subject:  user.Name,
		Issuer:   issuerClaimValue,
		Expiry:   jwt.NewNumericDate(time.Now().Add(t.expiration * time.Second)),
		IssuedAt: jwt.NewNumericDate(time.Now()),
	}
	// venice custom claims
	if privateClaims == nil {
		privateClaims = make(map[string]interface{})
	}
	privateClaims[TenantClaim] = user.GetTenant()
	privateClaims[RolesClaim] = user.Status.GetRoles()
	// create signed JWT
	token, err := jwt.Signed(t.signer).Claims(claims).Claims(privateClaims).CompactSerialize()
	if err != nil {
		log.Errorf("Unable to create JWT token: Err: %v", err)
		return "", err
	}
	return token, err
}

// Get returns object stored in session for the given key
// Returns
//   object
//   true if object exists otherwise false
func (t *jwtMgr) Get(token, key string) (interface{}, bool, error) {
	if !validateTokenFormat(token) {
		return nil, false, ErrInvalidTokenFormat
	}
	standardClaims, privateClaims, ok, err := t.validateJWTToken(fmt.Sprintf("%s.%s", t.header, token))
	if !ok {
		return nil, ok, err
	}
	// check key in standard and private claims
	switch key {
	case IssuerClaim:
		return standardClaims.Issuer, true, nil
	case SubClaim:
		return standardClaims.Subject, true, nil
	case ExpClaim:
		return standardClaims.Expiry, true, nil
	case IssuedAtClaim:
		return standardClaims.IssuedAt, true, nil
	case AudienceClaim, NotBeforeClaim, IDClaim:
		return nil, false, nil
	default:
		val, ok := privateClaims[key]
		return val, ok, nil
	}
}

// ValidateToken validates headless JWT token. It re-creates JWT header with alg as HS512 and prepends it to the headless token
// before doing validation
func (t *jwtMgr) ValidateToken(token string) (map[string]interface{}, bool, error) {
	if !validateTokenFormat(token) {
		return nil, false, ErrInvalidTokenFormat
	}
	standardClaims, privateClaims, ok, err := t.validateJWTToken(fmt.Sprintf("%s.%s", t.header, token))
	if !ok {
		return nil, ok, err
	}
	// collect all claims in a map
	privateClaims[SubClaim] = standardClaims.Subject
	privateClaims[IssuerClaim] = standardClaims.Issuer
	privateClaims[IssuedAtClaim] = standardClaims.IssuedAt
	privateClaims[ExpClaim] = standardClaims.Expiry

	return privateClaims, true, nil
}

// validateJWTToken validates JWT token. It checks algorithm in signature is HS512. It validates expiration and issuer claim.
func (t *jwtMgr) validateJWTToken(token string) (*jwt.Claims, map[string]interface{}, bool, error) {
	tok, err := jwt.ParseSigned(token)
	if err != nil {
		log.Errorf("Unable to parse JWT token: Err: %v", err)
		return nil, nil, false, err
	}
	// there should be only one signature
	if len(tok.Headers) != 1 {
		log.Errorf("Multiple signatures present in JWT")
		return nil, nil, false, ErrInvalidSignature
	}
	// signature algorithm type should be HS512
	if jose.SignatureAlgorithm(tok.Headers[0].Algorithm) != signatureAlgorithm {
		log.Errorf("Incorrect signature algorithm type")
		return nil, nil, false, ErrInvalidSignature
	}
	// standard jwt claims like sub, iss, exp
	standardClaims := jwt.Claims{}
	// venice custom claims
	privateClaims := make(map[string]interface{})
	if err := tok.Claims(t.secret, &standardClaims, &privateClaims); err != nil {
		log.Errorf("Unable to parse claims in JWT token: Err: %v", err)
		return nil, nil, false, err
	}
	// check if token is not expired and has correct issuer
	if err := standardClaims.Validate(jwt.Expected{Issuer: issuerClaimValue, Time: time.Now()}); err != nil {
		return nil, nil, false, err
	}

	return &standardClaims, privateClaims, true, nil
}

// removeJWTHeader removes header from JWT token. We will send headless JWT token to browser and recreate header while verifying token
// on the server. This is to make sure we control the algorithm type used for signing and don't accept insecure ones like "none" as alg type.
func removeJWTHeader(token string) string {
	parts := strings.Split(token, ".")
	return fmt.Sprintf("%s.%s", parts[1], parts[2])
}

// createJWTHeader creates JWT header. We will send headless JWT token to browser and recreate header while verifying token
// on the server. This is to make sure we control the algorithm type used for signing and don't accept insecure ones like "none" as alg type.
func createJWTHeader() (string, error) {
	header := &jwtHeader{
		Algorithm: headerAlgorithmValue,
		Type:      headerTypeValue,
	}
	headerBytes, err := json.Marshal(header)
	if err != nil {
		log.Errorf("Unable to marshal JWT header: Err: %v", err)
		return "", err
	}
	return base64.RawURLEncoding.EncodeToString(headerBytes), nil
}

func validateTokenFormat(token string) bool {
	parts := strings.Split(token, ".")
	if len(parts) != 2 {
		log.Error("Unexpected number of parts in JWT token")
		return false
	}
	return true
}
