# Capri A72 eSecure-loaded Non-Secure Boot Loader
This tree contains a trivial boot loader that will be installed on production Capri cards with eSecure enabled.

## Tree Layout
    include/                 Bootloader includes and defines
    src/                     Bootloader source
    test_keys/               Anil's eSecure test keys (for use with test-enabled boards)
    utils/                   eSecure python utils for code signing
    
## Building
This code is not intended to be part of a daily build.  It will likely only be built
and signed once in the lifetime of the company.

Build from the nic tree container.
The /tool/toolchain/aarch64-1.1 compiler is used, as for other nic tree code.

Typing 'make' in the src/ directory will build the boot_nonsec.bin.

Typing 'make signed' will also sign the image with the test_keys.  This requires some python packages not present in the
nic container.  Use pip2 to install:

    $ pip2 install --user pycrypto
    $ pip2 install --user ed25519
    $ pip2 install --user ecdsa
    $ make signed
    SIGN boot_nonsec.img
    SIGN+ENCRYPT boot_nonsec.img-encrypted

Two files are created:
- boot_nonsec.img is the image signed with the test keys but not encrypted.
- boot_nonsec.img-encrypted is both signed and encrypted.

# Theory of Operation
Production cards will have their eSecure modules initialized with final production keys, and when the eSecure boots
it will look for a signed A72 boot image to load into secure SRAM prior to taking the A72 out of reset.

The A72 image needs to do three things:
1. Check eFuses to ensure that Capri is in non-secure mode.  Stop if any fuses are blown.
2. Acknowledge to eSecure that the boot is successful.
3. Continue to the non-secure U-Boot in QSPI.

The main() function is essentially:

    if (is_secure_boot_enabled()) {
        panic("secure boot not supported");
    }
    esec_mark_boot_success();
    ((void (*)(void))ADDR_FLASH_BOOT)();

## Anti-Rollback Protection
Future Capri cards will support full secure boot through the whole boot chain, via an ARM Trusted Firmware boot flow.
When this boot code set is installed, whether during manufacturing or via an in-field software update, an obvious attack
would be to replace the eSecure-loaded A72 first-stage boot with this boot_nonsec image.  This image continues the boot
process without verifiying signatures, and thus turns a secure card into a non-secure card.

To prevent that, this code tests eFuse bits [511:508] (via is_secure_boot_enabled()), and only proceeds if all bits
are zero.  Multiple fuse bits are used to thwart FIB attack (see the Capri Crypto Hardware Architecture doc).  When we move
cards to full secure boot we will blow all four bits to 1.  This step irreversibly prevents boot_nonsec from boot a card, so
eliminating the rollback attack.

## Acknowledging eSecure
The A72 image that eSecure loads is required to send a mailbox command to acknowledge to eSecure that the A72 boot has been
successful.  The boot_nonsec code does this (via esec_mark_boot_success()) by sending a BootSuccess (command 0x07040000).

## Continuing to U-Boot
Finally, boot_nonsec jumps to our standard QSPI-resident U-Boot image at 0x70100000.  This will not return.
