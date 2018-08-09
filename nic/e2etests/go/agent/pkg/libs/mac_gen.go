package libs

import (
	"crypto/rand"
	"fmt"
)

func GenMACAddresses(count int) ([]string, error) {
	var macAddresses []string
	for i := 0; i < count; i++ {
		b := make([]byte, 6)
		_, err := rand.Read(b)
		if err != nil {
			return nil, err
		}
		// HAL will complain if EP has a multicast MAC Address. This will ensure that we have unicast MAC
		b[0] = (b[0] | 2) & 0xfe
		mac := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])
		macAddresses = append(macAddresses, mac)
	}
	return macAddresses, nil
}
