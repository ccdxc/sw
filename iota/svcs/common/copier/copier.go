package copier

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"time"

	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"

	"github.com/pensando/sw/venice/utils/log"
)

// Copier implements all remote to/from copy functions
type Copier struct {
	SSHClientConfig *ssh.ClientConfig
	SSHClient       *ssh.Client
}

// NewCopier returns a new copier instance
func NewCopier(c *ssh.ClientConfig) *Copier {
	copier := &Copier{
		SSHClientConfig: c,
	}
	copier.SSHClientConfig.Timeout = (10 * time.Second)
	return copier
}

// NewCopierWithSSHClient returns a new copier instance
func NewCopierWithSSHClient(c *ssh.Client, cfg *ssh.ClientConfig) *Copier {
	copier := &Copier{
		SSHClient:       c,
		SSHClientConfig: cfg,
	}
	return copier
}

func (c *Copier) getSftp(ipPort string) (*sftp.Client, error) {

	var err error
	var sftpClient *sftp.Client

	connect := func() (*ssh.Client, error) {
		sshclient, err := ssh.Dial("tcp", ipPort, c.SSHClientConfig)
		if sshclient == nil || err != nil {
			log.Errorf("Copier | CopyFrom node %v failed, Err: %v", ipPort, err)
			return nil, err
		}
		return sshclient, nil
	}

	for i := 0; i < 3; i++ {

		if c.SSHClient == nil {
			c.SSHClient, err = connect()
			if err != nil {
				return nil, err
			}
		}

		waitCh := make(chan error, 1)
		go func() {
			sftpClient, err = sftp.NewClient(c.SSHClient)
			waitCh <- err
		}()

		select {
		case <-time.After(3 * time.Second):
		case <-waitCh:
		}
		if sftpClient == nil || err != nil {
			log.Errorf("Copier | CopyFrom node %v failed, Err: %v", ipPort, err)
			c.SSHClient = nil
			log.Printf("Retrying connection to %v", ipPort)
			continue
		}
		return sftpClient, nil
	}

	return nil, err
}

// CopyTo copies the source files to the remote destination
func (c *Copier) CopyTo(ipPort, dstDir string, files []string) error {
	var err error
	var sftpClient *sftp.Client

	if sftpClient, err = c.getSftp(ipPort); err != nil {
		return err
	}

	defer sftpClient.Close()
	// check if dst dir exists
	if _, err := sftpClient.Lstat(dstDir); err != nil {
		err = sftpClient.Mkdir(dstDir)
		if err != nil {
			log.Errorf("Copier | CopyTo failed to creating remote directory. | Dir: %v, Node: %v, Err: %v", dstDir, ipPort, err)
		}
	}
	for _, absSrcFile := range files {
		//// Preflight checks
		if len(absSrcFile) == 0 {
			// Nothing to do here
			continue
		}
		//
		//pool.Go(func() error {
		if _, err := os.Stat(absSrcFile); err != nil {
			log.Errorf("Copier | CopyTo could not find the file %v. Err: %v", absSrcFile, err)
			return err
		}
		_, srcFile := filepath.Split(absSrcFile)
		absDstFile := filepath.Join(dstDir, srcFile)
		log.Infof("Copier | CopyTo initiating copy... | Src: %v, Dst: %v, Node: %v", absSrcFile, absDstFile, ipPort)

		// Check for the validity of source file
		out, err := ioutil.ReadFile(absSrcFile)
		if err != nil {
			log.Errorf("Copier | CopyTo could not read the source file. | SrcFile: %v, Node: %v, Err: %v", absSrcFile, ipPort, err)
			return err
		}

		f, err := sftpClient.Create(absDstFile)
		if err != nil {
			log.Errorf("Copier | CopyTo failed to create remote file. | File: %v, Node: %v, Err: %v", absDstFile, ipPort, err)
			return err
		}
		f.Chmod(0766)

		if _, err := f.Write(out); err != nil {
			log.Errorf("Copier | CopyTo failed to write the remote file. | File: %v, Node: %v, Err: %v", absDstFile, ipPort, err)
			return err
		}
		f.Close()
	}

	return nil

}

// CopyFrom copies the remote source files to the local destination
func (c *Copier) CopyFrom(ipPort, dstDir string, files []string) error {

	var err error
	var sftpClient *sftp.Client

	if sftpClient, err = c.getSftp(ipPort); err != nil {
		return err
	}

	defer sftpClient.Close()
	// check if dst dir exists
	if _, err := os.Lstat(dstDir); err != nil {
		err = os.Mkdir(dstDir, 0666)
		if err != nil {
			log.Errorf("Copier | CopyFrom failed to creating local directory. | Dir: %v,  Err: %v", dstDir, err)
			return err
		}
	}
	for _, srcFile := range files {

		absSrcFile, err := sftpClient.Open(srcFile)
		if err != nil {
			log.Errorf("Copier | CopyFrom failed to open remote file . | file: %v,  Err: %v", srcFile, err)
			return err
		}
		// Create the destination file
		_, srcFileName := filepath.Split(srcFile)
		dstFile := dstDir + "/" + srcFileName
		absDstFile, err := os.Create(dstFile)
		if err != nil {
			log.Errorf("Copier | CopyFrom failed to create local file. | Dir: %v, file : %v,  Err: %v", dstDir, dstFile, err)
			return err
		}

		log.Infof("Copier | CopyFrom initiating copy... | Src: %v, Dst: %v, Node: %v", srcFile, dstFile, ipPort)
		if _, err := absSrcFile.WriteTo(absDstFile); err != nil {
			log.Errorf("Copier | CopyFrom failed to copy local | Src: %v, Dst: %v, Node: %v", srcFile, dstFile, ipPort)
			return err
		}

		absSrcFile.Chmod(0766)
		absSrcFile.Close()
		absDstFile.Close()
	}

	return nil

}
