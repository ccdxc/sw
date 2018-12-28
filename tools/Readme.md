# Build tools

## How to publish new vagrant box image
There are two box images we use: 1) dev-vm 2) production vm (aka turin)

In order to customize a VM, you must install packer on your Mac/Linux system, using
the instructions specified [here](https://www.packer.io/intro/getting-started/setup.html).
For Mac, if you have `homebrew` installed, it is as easy as running `brew install packer`.

Following sub-sections specifies the steps to customize the vbox using packer.

### 1. Edit provisioning script

Edit `tools/pens-packer-scripts/pens.sh` (for dev-vm) or `tools/pens-packer-scripts/pens_turin.sh` (for turin) script and add the new provisioning
steps into the script.

### 2. build new vagrant image

```
# for dev-vm
make dev-build-centos74

# for turin
make turin-centos74
```

### 3. Try new vagrant image

change `tools/vagrant-files/Vagrantfile.dev` to use newly built image `venice/centos74` version `0` and bring up a vagrant VM and verify everything works in the VM.

### 4. Publish new image into Vagrant cloud

Go to `https://app.vagrantup.com` and login as `venice`. Ask @abhi or @jainvipin for login credentials.
Create a new version for `venice/centos74` image and add a new virtualbox provider.
Upload `tools/packer-templates/centos-7.4-x86_64-virtualbox.box` created by step 2.
Click "Release the new version" to make new version visible.

### 5. Modify vagrant file to use new version

change `tools/vagrant-files/Vagrantfile.dev` to use new version and verify pulling new version from vagrant could works.

### 6. Submit pull request

Commit all changes to packer scripts and vagrant file and submit pull request.
