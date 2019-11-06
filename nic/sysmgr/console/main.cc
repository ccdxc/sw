#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define APP_START "/sysconfig/config0/app-start.conf"
#define CONSOLE_MAGIC_FILE "/sysconfig/config0/.console"

#define HOSTPIN    "hostpin"
#define HOSTPIN_SZ sizeof(HOSTPIN)

#define CLASSIC_MODE 0
#define HOSTPIN_MODE 1

#define BUFLEN 32

// Returns HOSTPIN_MODE if we are in hostpin mode. CLASSICE_MODE if not
// We are in HOSTPIN_MODE *only* if APP_START file exists and it's contents are
// "hostpin"
static int nic_mode()
{
    int  fd;
    char buffer[HOSTPIN_SZ] = {0};
    int  n;

    fd = open(APP_START, O_RDONLY);
    if (fd == -1)
    {
        return CLASSIC_MODE;
    }

    n = read(fd, buffer, BUFLEN);
    close(fd);
    
    if (n == -1)
    {
        return CLASSIC_MODE;
    }

    // We don't care about the trailing \0 as long as what we read starts
    // with 'hostpin' we are good
    if (strncmp(HOSTPIN, buffer, HOSTPIN_SZ - 1) == 0)
    {
        return HOSTPIN_MODE;
    }

    return CLASSIC_MODE;
}

void
wait_enter (void)
{
    char c;
    while (1) {
        if (read(STDIN_FILENO, &c, 1) < 1) {
            exit(0);
        }
        if (c == '\n') {
            return;
        }
    }
}

void
print_string (const char *str)
{
    fputs(str, stdout);
    fflush(NULL);
}

int
main (int argc, char *argv[])
{
    const char *auth[] = {"/bin/login", NULL};
    const char *no_auth[] = {"/bin/login", "-f", "root", NULL};

    if (access(CONSOLE_MAGIC_FILE, F_OK) != -1)
    {
        // Console magic file is there, drop straight into bash shell
        execvp(no_auth[0], (char* const*)no_auth);
    }
    else if (nic_mode() == HOSTPIN_MODE)
    {
        // In network managed mode. Disable console
        while (1) {
            print_string("Console disabled\n");
            wait_enter();
        }
    }
    else
    {
        // Not in network managed mode, prompt for username/password
        print_string("Press enter to activate console\n");
        wait_enter();
        execvp(auth[0], (char* const*)auth);
    }   
}
