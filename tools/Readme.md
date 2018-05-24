# Build tools

## How to publish new vagrant box image

### 1. Edit provisioning script

Edit `tools/pens-packer-scripts/pens.sh` script and add the new provisioning
steps into the script.

### 2. build new vagrant image

```
make dev-build-centos74
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
