#include <iostream>

#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    FILE   *logfile;
    char   *filename;
    time_t now;

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
        sleep(15);
    }
}
