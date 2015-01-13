#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "record_file.h"
#include "index_file.h"

namespace storage
{

RecordFile::RecordFile(string base_name, uint32_t number)
: base_name_(base_name), number_(number), stream_id_(1 << 32 -1), locked_(false),
used_(false), record_fragment_count_(0), start_time_(0), end_time_(0), record_offset_(0)
{

}

int32_t RecordFile::ClearIndex()
{
    int ret;
    uint32_t offset;
    FILE *file = NULL;
    string index_file;

    Log(logger_, "clear index");

    index_file = base_name_ + "index";

    file = fopen(index_file.c_str(), "w");
    assert(file != NULL);

    struct RecordFileInfo record_file_info;
    memset(&record_file_info, 0, sizeof(struct RecordFileInfo));

    offset = number_ * sizeof(struct RecordFileInfo);

    ret = fseek(file, offset, SEEK_SET);
    assert(ret == 0);

    ret = fwrite((void *)record_file_info, 1, sizeof(struct RecordFileInfo), file);
    if (ret < sizeof(struct RecordFileInfo))
    {
        Log(logger_, "fwrite: ret < sizeof(struct RecordFileInfo), ret is %d", ret);
        assert(ret == sizeof(struct RecordFileInfo));
    }

    fflush(file);
    fclose(file);

    return 0;
}
}
