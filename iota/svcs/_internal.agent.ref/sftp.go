// +build ignore

package agent

import (
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/sftp"
	"github.com/sirupsen/logrus"
	"golang.org/x/crypto/ssh"
)

const (
	sshFxFailure = uint32(4)
	retryCount   = 10
	retryDelay   = time.Second
	sftpPort     = 22
)

// NewSFTPClientWithPublicKey creates a new sftp client
func NewSFTPClientWithPublicKey(ip, user, publicKeyFile string) (*sftp.Client, error) {
	//var hostKey ssh.PublicKey
	// A public key may be used to authenticate against the remote
	// server by using an unencrypted PEM-encoded private key file.
	//
	// If you have an encrypted private key, the crypto/x509 package
	// can be used to decrypt it.
	key, err := ioutil.ReadFile(publicKeyFile)
	if err != nil {
		log.Fatalf("unable to read private key: %v", err)
	}
	// Create the Signer for this private key.
	signer, err := ssh.ParsePrivateKey(key)
	if err != nil {
		log.Fatalf("unable to parse private key: %v", err)
	}

	config := &ssh.ClientConfig{
		User: user,
		//Auth: []ssh.AuthMethod{
		//	ssh.Password(pwd),
		//},
		Auth: []ssh.AuthMethod{
			// Use the PublicKeys method for remote authentication.
			ssh.PublicKeys(signer),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	return sftpConnect(ip, sftpPort, config)
}

// NewSFTPClient creates a new sftp client
func NewSFTPClient(ip, user, pwd string) (*sftp.Client, error) {

	config := &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(pwd),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	return sftpConnect(ip, sftpPort, config)
}

// NewSFTPClientWithPort creates a new sftp client
func NewSFTPClientWithPort(ip string, port int, user, pwd string) (*sftp.Client, error) {

	config := &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(pwd),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	return sftpConnect(ip, port, config)
}

func sftpConnect(ip string, port int, config *ssh.ClientConfig) (*sftp.Client, error) {
	var sshErr error
	var sshc *ssh.Client

	for ix := 0; ix < retryCount; ix++ {
		sshc, sshErr = ssh.Dial("tcp", ip+":"+strconv.Itoa(port), config)
		if sshErr == nil {
			break
		}

		time.Sleep(retryDelay)
	}

	if sshErr != nil {
		return nil, sshErr
	}

	client, err := sftp.NewClient(sshc)
	if err != nil {
		logrus.Error(err)
		return nil, err
	}

	return client, nil
}

// mkdirParents equivalent of mkdir -p
func mkdirParents(client *sftp.Client, dir string) error {
	var parents string
	var err error

	if path.IsAbs(dir) {
		// Otherwise, an absolute path given below would be turned in to a relative one
		// by splitting on "/"
		parents = "/"
	}

	for _, name := range strings.Split(dir, "/") {
		if name == "" {
			// Paths with double-/ in them should just move along
			// this will also catch the case of the first character being a "/", i.e. an absolute path
			continue
		}
		parents = path.Join(parents, name)
		err = client.Mkdir(parents)
		if status, ok := err.(*sftp.StatusError); ok {
			if status.Code == sshFxFailure {
				var fi os.FileInfo
				fi, err = client.Stat(parents)
				if err == nil {
					if !fi.IsDir() {
						return fmt.Errorf("File exists: %s", parents)
					}
				}
			}
		}
		if err != nil {
			break
		}
	}
	return err
}

// sftpSingle copies recursively to the specified dest
func sftpSingle(srcPath, rPath string, remote *sftp.Client, respCh chan error) {
	var lErr error

	// ensure rPath exists
	err := mkdirParents(remote, rPath)
	if err != nil {
		respCh <- fmt.Errorf("Failed to create %s", rPath)
		return
	}
	info, err := os.Stat(srcPath)
	if err != nil {
		respCh <- fmt.Errorf("Failed to access %s", srcPath)
		return
	}

	dest := filepath.Join(rPath, filepath.Base(srcPath))
	lErr = copy(srcPath, dest, info, remote)

	respCh <- lErr
}

// copy copies a file or dir to dest
func copy(srcPath, dest string, info os.FileInfo, remote *sftp.Client) error {
	if info.IsDir() {
		if info.Mode()&os.ModeSymlink != 0 {
			return symLinkCopy(srcPath, dest, remote)
		}
		return dirCopy(srcPath, dest, remote)
	}

	return fileCopy(srcPath, dest, info, remote)
}

// fileCopy copies a single file to the dest
func fileCopy(srcPath, dest string, info os.FileInfo, remote *sftp.Client) error {
	if !info.Mode().IsRegular() {
		if info.Mode()&os.ModeSymlink != 0 {
			return symLinkCopy(srcPath, dest, remote)
		}
		logrus.Warnf("Skipping irregular file %s", srcPath)
		return nil
	}

	s, err := os.Open(srcPath)
	if err != nil {
		logrus.Errorf("err: %v fileinfo: %+v", err, info)
		return err
	}
	defer s.Close()

	d, err := remote.Create(dest)
	if err != nil {
		return err
	}
	defer d.Close()

	err = d.Chmod(info.Mode().Perm())
	if err != nil {
		logrus.Errorf("chmod err: %v fileinfo: %+v", err, info)
		return err
	}

	count, err := io.Copy(d, s)
	if err != nil {
		return err
	}

	if count != int64(info.Size()) {
		// could hit this if we had a symlink
		logrus.Warnf("copy %s to %s expected %v, got %v", srcPath, dest, info.Size(), count)
	}
	return nil
}

// dirCopy creates the directory on the dest and copies contents
// recursively
func dirCopy(srcPath, dest string, remote *sftp.Client) error {
	err := remote.Mkdir(dest)
	if err != nil {
		return err
	}
	infos, err := ioutil.ReadDir(srcPath)
	if err != nil {
		logrus.Errorf("err: %v fileinfo: %+v", err, infos)
		return err
	}

	for _, info := range infos {
		if err := copy(
			filepath.Join(srcPath, info.Name()),
			filepath.Join(dest, info.Name()),
			info,
			remote,
		); err != nil {
			return err
		}
	}

	return nil
}

func getRelativeLink(from, to string) string {
	if !filepath.IsAbs(to) {
		// already a relative link
		return to
	}

	base := filepath.Dir(from)
	rel, err := filepath.Rel(base, to)
	if err == nil {
		return rel
	}

	return to // use the absolute path
}

func symLinkCopy(srcPath, dest string, remote *sftp.Client) error {
	var linkTo string

	link, err := filepath.EvalSymlinks(srcPath)
	if err != nil {

		linkTo, err = os.Readlink(srcPath)
		if err != nil {
			logrus.Warnf("Cannot resolve symlink %s - %v", srcPath, err)
			return nil
		}
		logrus.Infof("Src %s is a dead link %s", srcPath, linkTo)
	} else {

		logrus.Infof("link %s resolve to %s", srcPath, link)
		linkTo = getRelativeLink(dest, link)
	}
	err = remote.Symlink(linkTo, dest)
	if err != nil {
		logrus.Errorf("symlink %v to %v - err: %v", linkTo, dest, err)
	} else {
		logrus.Infof("Added symlink %s -> %s", dest, linkTo)
	}
	return nil
}

// MultiSFTP sftp's the source to multiple remotes
func MultiSFTP(srcPath, rPath string, remotes []*sftp.Client) error {
	_, err := os.Stat(srcPath)
	if err != nil {
		return err
	}

	respCh := make(chan error, len(remotes))
	// spawn sftp to each remote
	for _, remote := range remotes {
		go sftpSingle(srcPath, rPath, remote, respCh)
	}

	errStr := ""
	// process results
	for range remotes {
		resp := <-respCh
		if resp != nil {
			errStr += resp.Error() + "; "
		}
	}

	if errStr != "" {
		return errors.New(errStr)
	}

	if len(remotes) != 0 {
		logrus.Infof("sftp::Copied %s to %s on %v instances", srcPath, rPath, len(remotes))
	}
	return nil
}
