
## Packer files for building VMs
The VMs used for testing, development are built using packer tool.
More documentation on Packer can be found [here](https://www.packer.io/docs)

### Customizing VMs with packer
In order to customize a VM, you must install packer on your Mac/Linux system, using
the instructions specified [here](https://www.packer.io/intro/getting-started/setup.html).
For Mac, if you have `homebrew` installed, it is as easy as running `brew install packer`.

Following sub-sections specifies the steps to customize the vbox using packer.

`packer-templates` is original work of @kaorimatz (thanks!); this repo contains a copy of
[this repository](https://github.com/kaorimatz/packer-templates).

#### 1. Modify an existing box image
Likely you would modify an existing box image to include a few more binaries/packages. If yes,
you can do it simply by making the changes to the shell script in `scripts/centos7.3/pnsd.sh`,
for example, and run the make target:
```
make dev-build-centos73
```

#### 2. Creating a new box image
These instructions provide you with creating a new box image from scratch.

##### 2.1 Choose the target OS distro 
Pick a target distro file in `packer-templates/*.json` that matches the target environment
you need to make changes to, for example, Ubuntu16.10, Centos7.2, etc.
If adding a new custom script, edit the json file to add `pnsd` custom script location. 
See `packer-templates/centos-7.2-x86_64.json` for example.
If the new scripts need to be added, it could be located in `packer-templates/scripts/<distro>`
directory. See `packer-templates/scripts/centos-7.2/pnsd.sh` for example.

##### 2.2 Create a make target for the Box image 
Add following commands in the Makefile target in `tools/Makefile` for the new
box image, for example for `dev-build-centos73` target has following two commands
```
dev-build-centos73:
	cd packer-templates; packer build -only=virtualbox-iso centos-7.3-x86_64.json
	cd packer-templates; vagrant box add jainvipin/centos73 centos-7.3-x86_64-virtualbox.box
```

#### 3. Verify your changes
By running the command `make dev-test` you can verify if you didn't
break any of the existing work with your changes to the packer template.
However, you must add new tests in `scripts/CI.sh` file to add tests for the functionality
you added to the new box that was created. Feel free to look at some existing tests
and replicate.

##### 4. Submit a Pull Request
After verifying your changes and adding new test cases and submit a PR.
After PR is approved, the box needs to be manually built and uploaded to
Atlas (web portal to host vagrant boxes). Please contact @jainvipin for this step
(until a paid atlas account is setup), which can enable using `packer push` and automate this step.

##### 5. Updating Centos distro
It is likely the iso images pointed to by the database here is outdated and you may have to
update the centos disro in order to proceed further. The steps to upgrade would be:
- Update the packer-templates
`git clone https://github.com/kaorimatz/packer-templates`
- Run following script that fixes up the newly checked out repo
`$ ./pens-packer-scripts//packer-fixup.sh`
- Add pens.sh to apprpiriate packer template json file, for example `packer-templates/centos-7.3-x86_64.json`
```
 . . .
    "scripts": [
      "scripts/centos-7.3/repo.sh",
      "scripts/centos/virtualbox.sh",
      "scripts/centos/pens.sh",				# <== add this line
      "scripts/centos/vmware.sh",
 . . .
```
- You may have to change the centos7x targets in Makefile
