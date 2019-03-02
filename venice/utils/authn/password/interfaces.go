package password

// Hasher is an abstraction to hide crypto algorithm used to generate password hash using a salt. We could have  bcrypt, scrypt, argon2 based implementations.
type Hasher interface {
	//GetPasswordHash generates a salted hash given the password
	GetPasswordHash(password string) (string, error)

	//CompareHashAndPassword compares password hash with its plaintext equivalent
	CompareHashAndPassword(hash string, password string) (bool, error)
}

// PolicyChecker checks if password satisfies password policy
type PolicyChecker interface {
	// Validate validates password against password policy
	Validate(password string) []error
}
