
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
you can do it simply by making the changes to the shell script in `scripts/centos7.2/nvmt_net.sh`,
for example, and run the make target:
```
make dev-build-centos72
```

#### 2. Creating a new box image
These instructions provide you with creating a new box image from scratch.

##### 2.1 Choose the target OS distro 
Pick a target distro file in `packer-templates/*.json` that matches the target environment
you need to make changes to, for example, Ubuntu16.10, Centos7.2, etc.
If adding a new custom script, edit the json file to add `nvmt` custom script location. 
See `packer-templates/centos-7.2-x86_64.json` for example.
If the new scripts need to be added, it could be located in `packer-templates/scripts/<distro>`
directory. See `packer-templates/scripts/centos-7.2/nvmt_net.sh` for example.

##### 2.2 Create a make target for the Box image 
Add following commands in the Makefile target in `tools/Makefile` for the new
box image, for example for `dev-build-centos72` target has following two commands
```
dev-build-centos72:
	cd packer-templates; packer build -only=virtualbox-iso centos-7.2-x86_64.json
	cd packer-templates; vagrant box add jainvipin/centos72 centos-7.2-x86_64-virtualbox.box
```

#### 3. Verify your changes
By running the command `make dev-test` you can verify if you didn't
break any of the existing work with your changes to the packer template.
However, you must add new tests in `scripts/CI.sh` file to add tests for the functionality
you added to the new box that was created. Feel free to look at some existing tests
and replicate.

##### 4. Submit a Pull Request
After verifying your changes and adding new test cases, submit a PR against `tools` repository.
After PR is approved, the box needs to be manually built and uploaded to
Atlas (web portal to host vagrant boxes). Please contact @jainvipin for this step
(until a paid atlas account is setup), which can enable using `packer push` and automate this step.
