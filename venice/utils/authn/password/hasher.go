package password

import (
	"golang.org/x/crypto/bcrypt"
)

type bcryptHasher struct {
	cost int
}

func newBcryptHasher() Hasher {
	return &bcryptHasher{cost: 12} //TODO determine cost based on time it takes for typical venice server to generate/verify hash; should be ~240ms
}

//GetPasswordHash generates a salted hash given the password
func (bh *bcryptHasher) GetPasswordHash(password string) (string, error) {
	hash, err := bcrypt.GenerateFromPassword([]byte(password), bh.cost)
	return string(hash), err
}

//CompareHashAndPassword compares password hash with its plaintext equivalent
func (bh *bcryptHasher) CompareHashAndPassword(hash string, password string) (bool, error) {
	if err := bcrypt.CompareHashAndPassword([]byte(hash), []byte(password)); err != nil {
		return false, err
	}
	return true, nil
}

//GetPasswordHasher returns Hasher to hash password. It currently returns bcrypt based implementation.
func GetPasswordHasher() Hasher {
	return newBcryptHasher()
}
