/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <iostream>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "platform/pal/include/pal.h"

#define FLASH_READ_FILENAME "/tmp/readflash.bin"
#define CPLDAPP_LOCK "/var/lock/cpldapp_lock"
#define MAX_WRITE_RETRY 5
#define MIN_PERCENT_LOG 10

using namespace std;

int applock = -1;

static int
lock_cpldapp(void)
{
    struct flock lock = {F_WRLCK, 0, SEEK_SET, 0};
    if (fcntl(applock, F_SETLKW, &lock) == -1) {
        cerr << "Error locking the app" << endl;
        return -1;
    }
    return 0;
}

static int
unlock_cpldapp(int status)
{
    struct flock unlock = {F_UNLCK, 0, SEEK_SET, 0};
    if (fcntl(applock, F_SETLKW, &unlock) == -1) {
        cerr << "Error unlocking the app" << endl;
        return -1;
    }
    return status;
}

void
printstatus(pal_cpld_status_t status, int percentage, void *ctxt)
{
    string stat;
    static int previous_percentage;
    switch(status) {
        case PAL_UPGRADE_COMPLETED:
            stat = "Write completed";
            break;
        case PAL_UPGRADE_ERASED_DONE:
            stat = "flash erased";
            break;
        case PAL_UPGRADE_WRITE_DONE:
            stat = "flash written";
            break;
        case PAL_UPGRADE_VERIFY_DONE:
            stat = "flash verified";
            break;
        default:
            return;
    }

    if (percentage % MIN_PERCENT_LOG == 0 && previous_percentage != percentage) {
        cout << "---Cpld upgrade---" << stat << " "\
            << percentage << "% done" << endl;
        previous_percentage = percentage;
    }

}

static int
usage(void)
{
    cerr << "cpldapp (-reload) - Issues a CPLD reload." << endl;
    cerr << "cpldapp (-qsfpportpresent) - Displays the port status." << endl;
    cerr << "cpldapp (-verifyid) - Verifies the id of the cpld." << endl;
    cerr << "cpldapp (-erase) - Erases the current CPLD image." << endl;
    cerr << "cpldapp (-r <addr>) - Reads the cpld register." << endl;
    cerr << "cpldapp (-verifyflash <filename>) - Reads the flash and verifies against the file provided." << endl;
    cerr << "cpldapp (-writeflash <filename>) - write the flash with the file provided." << endl;
    cerr << "cpldapp (-hwlock) - checks whether cpld is hwlock enabled or not." << endl;
    cerr << "cpldapp (-alompresent) - checks whether alom is present or not(only for swm cards)." << endl;
    return unlock_cpldapp(-1);
}

int
verifyflash(char *filename) {
    uint8_t *buf;
    uint8_t *cpldbuf;
    uint32_t numbytes;
    FILE *readflashfile;
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        cerr << "Cannot open the file " << filename<< endl;
        return -1;
    }

    // Get the number of byte
    fseek(infile, 0, SEEK_END);
    numbytes = ftell(infile);
    // reset the file position indicator to
    // the beginning of the file
    fseek(infile, 0, SEEK_SET);
    cout << "Length of the file is " << numbytes << endl;

    buf = (uint8_t*)calloc(numbytes, sizeof(uint8_t));
    // copy all the data into the buffer
    fread(buf, sizeof(uint8_t), numbytes, infile);
    fclose(infile);

    cpldbuf = (uint8_t*)calloc(numbytes, sizeof(uint8_t));
    if (pal_cpld_read_flash(cpldbuf, numbytes) == 0) {
        readflashfile = fopen(FLASH_READ_FILENAME, "wb");
        if (readflashfile == NULL) {
            cerr << "Cannot create file " << FLASH_READ_FILENAME << endl;
        } else {
            fwrite(cpldbuf, numbytes, 1, readflashfile);
            fclose(readflashfile);
        }
        if (memcmp(buf, cpldbuf, numbytes) == 0) {
            cout << "CPLD Verified" << endl;
        } else {
            cerr << "CPLD doesnt match" << endl;
            free(buf);
            free(cpldbuf);
            return -1;
        }
        free(buf);
        free(cpldbuf);
        return 0;
    }

    cerr << "Unable to read cpld, cannot verify" << endl;
    free(buf);
    free(cpldbuf);
    return -1;

}

int
writeflash(char *filename) {
    uint8_t *buf;
    uint32_t numbytes;
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        cerr << "Cannot open the file " << filename<< endl;
        return -1;
    }

    // Get the number of byte
    fseek(infile, 0, SEEK_END);
    numbytes = ftell(infile);
    // reset the file position indicator to
    // the beginning of the file
    fseek(infile, 0, SEEK_SET);
    cout << "Length of the file is " << numbytes << endl;

    buf = (uint8_t*)calloc(numbytes, sizeof(uint8_t));
    // copy all the data into the buffer
    fread(buf, sizeof(uint8_t), numbytes, infile);
    fclose(infile);

    if (pal_cpld_write_flash(buf, numbytes, &printstatus, NULL) == 0) {
        cout << "CPLD written" << endl;
        free(buf);
        return 0;
    }
    cerr << "Unable to write cpld" << endl;
    free(buf);
    return -1;
}

int
main(int argc, char *argv[])
{
    applock = open(CPLDAPP_LOCK, O_CREAT | O_RDWR);
    if (applock < 0) {
        cerr << "Error opening the lock file" << endl;
        return -1;
    }
    if (lock_cpldapp() == -1) {
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    if (argc < 2) {
        return usage();
    }

    if (strcmp(argv[1], "-qsfpportpresent") == 0) {
        cout << "QSFP port 1 : " << pal_is_qsfp_port_psnt(1) << endl;
        cout << "QSFP port 2 : " << pal_is_qsfp_port_psnt(2) << endl;
    } else if (strcmp(argv[1], "-reload") == 0) {
        pal_cpld_reload_reset();
    } else if (strcmp(argv[1], "-verifyid") == 0) {
        cout << "CPLD " << (pal_cpld_verify_idcode() ? "verified" : "unable to verify") << endl;
    } else if (strcmp(argv[1], "-erase") == 0) {
        if (pal_cpld_erase() == 0) {
            cout << "CPLD erased. Do not reset without flashing a new image" << endl;
            goto success;
        } else {
            cerr << "Error erasing CPLD" << endl;
            return unlock_cpldapp(-1);
        }
    } else if (strcmp(argv[1], "-r") == 0) {
        if (argc < 3) {
            return usage();
        }
        cout << hex << showbase << cpld_reg_rd(strtoul(argv[2], NULL, 0)) << endl;
    } else if (strcmp(argv[1], "-verifyflash") == 0) {
        if (argc < 3) {
            usage();
        }
        if (verifyflash(argv[2])) {
            return unlock_cpldapp(-1);
        }
    } else if (strcmp(argv[1], "-writeflash") == 0) {
        if (argc < 3) {
            return usage();
        }
        for (int counter=0; counter < MAX_WRITE_RETRY; counter++) {
            if (writeflash(argv[2]) == 0) {
                goto success;
            }
        }
        return unlock_cpldapp(-1);
    } else if (strcmp(argv[1], "-hwlock") == 0) {
        cout << "CPLD hwlock is" << (pal_cpld_hwlock_enabled() ? "enabled" : "disabled") << endl;
    } else if (strcmp(argv[1], "-alompresent") == 0) {
        if (cpld_reg_rd(CPLD_REGISTER_ID) == CPLD_ID_NAPLES25_SWM) {
            if (cpld_reg_rd(CPLD_REGISTER_CTRL) & CPLD_ALOM_PRESENT_BIT) {
                cout << "ALOM present";
                goto success;
            }
        }
        cout << "ALOM not present";
    } else {
        return usage();
    }
success:
    return unlock_cpldapp(0);
}
