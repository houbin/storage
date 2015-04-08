#include <string>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "fill_index.h"
#include "../../util/coding.h"
#include "../../util/clock.h"
#include "../../util/crc32c.h"
#include "../../src/store_types.h"

using namespace std;
using namespace util;
using namespace storage;

int32_t IndexFileManager::ScanAllIndexFile()
{

    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;

    dp = opendir("/jovision/mnt/");
    assert(dp != NULL);

    chdir("/jovision/mnt/");
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            if ((strcmp(".", entry->d_name) == 0) 
                || (strcmp("..", entry->d_name) == 0))
            {
                continue;
            }

            IndexFile *index_file = NULL;
            string base_name("/jovision/mnt/");
            base_name = base_name + entry->d_name + "/";

            /* test if dir is legal */
            char buffer[32] = {0};
            int file_count = 0;
            string temp_file_count_file = base_name + "file_count";
            FILE *fp = NULL;
            fp = fopen(temp_file_count_file.c_str(), "r");
            if (fp == NULL)
            {
                continue;
            }
            else
            {
                fread(buffer, 1, 31, fp);
                file_count = atoi(buffer);
                fclose(fp);
                fp = NULL;
            }

            index_file = new IndexFile(base_name, file_count);
            assert(index_file != NULL);

            index_file_vec_.push_back(index_file);
        }
    }

    closedir(dp);

    return 0;
}

void FillRecordFileIndex(char *temp)
{
    uint32_t length;

    memset(temp, 0, sizeof(RecordFileInfo));

    length = sizeof(RecordFileInfo) - 4 - 4;
    EncodeFixed32(temp, length);
    temp += 4;
    
    /* skip crc */
    temp += 4;
    
    char *crc_start = temp;
    char temp_buffer[64] = {0};
    memset(temp_buffer, 'x', 10);
    memcpy(temp, temp_buffer, 64);
    temp += 64;

    *temp = 0;
    temp += 1;

    uint16_t record_frag_count = 2;
    *temp = record_frag_count & 0xff;
    *(temp + 1) = (record_frag_count >> 8) & 0xff;
    temp += 2;

    UTime now;
    now = GetClockNow();
    EncodeFixed32(temp, now.tv_sec);
    temp += 4;

    EncodeFixed32(temp, now.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, now.tv_sec + 100);
    temp += 4;

    EncodeFixed32(temp, now.tv_nsec);
    temp += 4;

    temp += 4;
    temp += 4;
    temp += 4;
    temp += 4;

    EncodeFixed32(temp, 10000);
    temp += 4;

    uint32_t crc = crc32c::Value(crc_start, length);
    EncodeFixed32(crc_start - 4, crc);

    return;
}

int main()
{
    IndexFileManager index_file_manager;
    index_file_manager.ScanAllIndexFile();

    vector<IndexFile*>::iterator iter = index_file_manager.index_file_vec_.begin();
    for (iter; iter != index_file_manager.index_file_vec_.end(); iter++)
    {
        IndexFile *index_file = *iter;
        fprintf(stderr, "index file %s, have %d record file\n", index_file->base_name.c_str(), index_file->file_count);
    }

    iter = index_file_manager.index_file_vec_.begin();
    for (iter; iter != index_file_manager.index_file_vec_.end(); iter++)
    {
        IndexFile *index_file = *iter;
        uint32_t file_count = index_file->file_count;
        string base_name = index_file->base_name;
        uint32_t read_length = sizeof(RecordFileInfo) * file_count;

        RecordFileInfo *buffer = NULL;
        buffer = new RecordFileInfo[file_count];
        assert(buffer != NULL);

        string index_name = base_name + "index";
        FILE *fp = fopen(index_name.c_str(), "r+");
        assert(fp != NULL);
        size_t ret = fread((void*)buffer, 1, read_length, fp);
        assert(ret == read_length);

        fprintf(stderr, "");
        fprintf(stderr, "start fwrite, base name %s, file count %d", base_name.c_str(), file_count);
        for (int i = 0; i < file_count; i++)
        {
            char *temp = (char *)(buffer + i);
            uint32_t length = DecodeFixed32(temp);
            if (length != 0)
            {
                continue;
            }

            RecordFileInfo record_file_info = {0};
            FillRecordFileIndex((char *)&record_file_info);
            fseek(fp, i * sizeof(RecordFileInfo), SEEK_SET);
            size_t ret = fwrite((void*)&record_file_info, 1, sizeof(RecordFileInfo), fp);
            if (ret != sizeof(RecordFileInfo))
            {
                fprintf(stderr, "fwrite error, error msg %s, ret is %d, sizeof RecordFileInfo %d", strerror(errno), ret, sizeof(RecordFileInfo));
            }
            fprintf(stderr, "fwrite ok, seq %d\n", i);
        }
    }

    return 0;
}
