# boot0 Loader

This directory builds boot0.bin, which is the "A/B U-Boot" pre-loader.

## Role
The boot0.bin program is executed from flash from 0x70100000.  Its purpose
is to select a u-boot image (A, B, or Gold) to execute based on user
selection or force-golden GPIO, and then to provide mechanisms for
auto-recovery of certain failure conditions, such as hangs, crashes,
and panics.

## Building
The program is built as part of a top-level nic/ build for aarch64.
```
nic$ make ARCH=aarch64 PLATFORM=hw boot0.submake
```
The program can also be built from this src/ directory by using the
runmake helper:
```
$ ./runmake
```
## Versioning
In the A/B U-Boot model, the (previously unsafe) u-boot image is
safely upgradeable just like the application kernel and filesystem.
An application "image set" then comprises u-boot (which configures the
NOC, HBM, and PCIE), the kernel image, and the application filesystem.
The purpose of boot0.bin is to be the stable starting point that selects
the image track to execute. and manages the boot-up sequence.

As the stable component, the boot0.bin program is not usually updated
in the field (as it cannot be fail safe), but there are scenarios
(e.g. critical bug fixes) for which this would be necessary.  For example,
if it is found that the QSPI flash is unstable on some boards at a certain
speed, then it would be necessary to update boot0.bin *on those boards
only*, in order to fend off a field problem.

To address this contingency, the program contains an embedded version
number that is compared against a per-board-type minimum.  If the
installed boot0.bin has a version number below this minimum, and a
newer boot0.bin is available during a firmware update, then the program
is updated.  Otherwise it is not.

More detail:
* Each hardware board type has a minimum boot0 version that must be installed.
* The mapping from board part number to minimum version is recorded in this directory in the file boards.json.
* A firmware update package (naples_fw.tar) includes both the latest boot0.bin, and the latest boards.json.
* During a firmware update, the version of the installed boot0.bin is compared against the board record in the boards.json carried in the naples_fw.tar.  If the boards.json says the board should be running a newer boot0.bin, and a newer version is included in the naples_fw.tar, then the boot0.bin is (non-fail-safe) updated as part of the upgrade.

Importantly, unless directed by boards.json, the boot0.bin on the board
will not be updated even if the naples_fw.tar content is newer (has a
higher version).

## Keeping the Version Updated
The role of the embedded version number is so important that the
Makefile directly supports verifying that the compiled result matches the
version number.  This is done via the version.txt file, which contains {
version, hash } of boot0.bin.  Any time boot0.bin changes (e.g. due to
a code change) the version.txt check will fail and you'll be prompted
to take action:

```
$ ./runmake 
CC main.c
LINK boot0
MAKEBIN boot0.bin
MAKELIST boot0.lst
###
### ERROR: boot0.bin does not match version 1.
### make new_version to increment the program version.
### make version.txt to keep the current version.
###
Makefile:79: recipe for target 'check_version' failed
make: *** [check_version] Error 1
```
The first option here is to increment the program's version number (in ../include/version.h) by issung:
```
$ ./runmake new_version

```
This creates a new ../include/version.h, one higher than before, re-builds
the program, and re-creates version.txt with the new hash of the image.
If instead you want to skip the version increment, you can just fix version.txt:
```
$ ./runmake version.txt
```
Remember to check-in the new ../include/version.h (if new_version)
and versions.txt.

## Specifying the Min Version for a Board
The boards.json file holds the minimum boot0.bin version required for
all boards.  Here is an example:
```
{
    "board_map": [
        { "part-number": "68-0004", "min_version": 2 },
        { "part-number": "68-0007", "min_version": 3 }
    ]
}
```
This file says that:
* Board part number 68-0004 requires a boot0.bin version >= 2.
* Board 68-0007 requires a boot0.bin version >= 3.
* All other boards require a boot0.bin version >= 0.

The expected workflow is that only when a field-update is actually
required will an entry be added for a specific board, and ideally this
table will be empty (indicating no field-updates).
