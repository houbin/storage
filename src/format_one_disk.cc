#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "../include/storage_api.h"
#include "../src/store_types.h"

using namespace std;
using namespace storage;

void usage(char *argv0)
{
    fprintf(stderr, "usage: %s [disk name]\n", argv0);

    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
        return -1;
    }

    char *disk = argv[1];

    int file_info_length = sizeof(RecordFileInfo);
    int frag_info_length = sizeof(RecordFragmentInfo);

    char command[256] = {0};
    snprintf(command, 255, "/jovision/shell/format_one_disk.sh %s %d %d", disk, file_info_length, frag_info_length);
    FILE *fp = NULL;
    fp = popen(command, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "popen shell error");
        return -1;
    }

    char *result_buffer = new char[2048];
    memset(result_buffer, 0, 2048);
    while(fgets(result_buffer, 2048, fp) != NULL)
    {
        
    }

    pclose(fp);
    delete []result_buffer;

    return 0;
}

