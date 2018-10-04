package copier

import (
	"io/ioutil"
	"path/filepath"

	"github.com/pkg/sftp"
	log "github.com/pensando/sw/venice/utils/log"
	"golang.org/x/crypto/ssh"
)

// Copier implements all remote to/from copy functions
type Copier struct {
	SSHClientConfig *ssh.ClientConfig
}

// NewCopier returns a new copier instance
func NewCopier(c *ssh.ClientConfig) *Copier {
	copier := &Copier{
		SSHClientConfig: c,
	}
	return copier
}

// Copy copies from the source file to a dest file on a remote node
func (c *Copier) Copy(ipPort, absSrcFile, absDstFile string) error {
	log.Infof("Copier | CopyTo initiating copy... | Src: %v, Dst: %v, Node: %v", absSrcFile, absDstFile, ipPort)
	//srcDir,  := filepath.Split(absSrcFile)
	dstDir, _ := filepath.Split(absDstFile)

	// Check for the validity of source file
	out, err := ioutil.ReadFile(absSrcFile)
	if err!= nil {
		log.Errorf("Copier | CopyTo could not read the source file. | SrcFile: %v, Node: %v, Err: %v", absSrcFile, ipPort, err)
		return err
	}

	client, err := ssh.Dial("tcp", ipPort, c.SSHClientConfig)
	if client == nil || err != nil {
		log.Errorf("Copier | CopyTo node %v failed, Err: %v", ipPort, err)
		return err
	}
	defer client.Close()

	sftp, err := sftp.NewClient(client)
	if sftp == nil || err != nil {
		log.Errorf("Copier | CopyTo node %v failed, Err: %v", ipPort, err)
		return err
	}
	defer sftp.Close()

	// check if dst dir exists
	if _, err := sftp.Lstat(dstDir); err != nil {
		err = sftp.Mkdir(dstDir)
		if err != nil {
			log.Errorf("Copier | CopyTo failed to creating remote directory. | Dir: %v, Node: %v, Err: %v", dstDir, ipPort, err)
		}
	}

	f, err := sftp.Create(absDstFile)
	if err != nil {
		log.Errorf("Copier | CopyTo failed to create remote file. | File: %v, Node: %v, Err: %v", absDstFile, ipPort, err)
		return err
	}
	f.Chmod(0766)

	if _, err := f.Write(out); err != nil {
		log.Errorf("Copier | CopyTo failed to write the remote file. | File: %v, Node: %v, Err: %v", absDstFile, ipPort, err)
		return err
	}

	defer f.Close()
	return nil
}
