#include <iostream>
#include <string>

#include <time.h>
#include <unistd.h>

/* LOG_SIZE is 1440 for 6 hour worth of logs. 1 log every 15 seconds */
#define LOG_SIZE 1440

int main(int argc, char *argv[])
{
    FILE   *logfile;
    char   *filename;
    time_t now;
    int    size = 0;

    if (argc < 2)
    {
        printf("Usage: %s <LOGFILENAME>", argv[0]);
        exit(-1);
    }

    filename = argv[1];
    logfile = fopen(filename, "w");
    if (logfile == NULL)
    {
        printf("Failed to open %s", filename);
    }
    
    while (true) {
        time(&now);
        fprintf(logfile, "%s", asctime(gmtime(&now)));
        fflush(logfile);
        size += 1;
        if (size > LOG_SIZE) {
            char old[256];
            snprintf(old, sizeof(old), "%s.1", filename);
            unlink(old);
            fclose(logfile);
            rename(filename, old);
            logfile = fopen(filename, "w");
            size = 0;
        }
        sleep(15);
    }
}
