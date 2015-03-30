#ifndef STORAGE_RECORD_READER_H_
#define STORAGE_RECORD_READER_H_

#include "../include/errcode.h"
#include "../include/storage_api.h"
#include "../util/utime.h"
#include "../util/logger.h"
#include "../util/thread.h"
#include "../util/rwlock.h"
#include "store_types.h"
#include "config_opts.h"
#include "id_center.h"
#include "free_file_table.h"

using namespace std;
using namespace util;

namespace storage
{

class RecordFile;
class RecordFileMap;

class RecordReader
{
private:
    Logger *logger_;
    RecordFileMap *file_map_;
    RecordFile *record_file_;

    uint32_t read_offset_;
    uint32_t read_end_offset_;

    FRAME_INFO_T current_o_frame_;

public:
    RecordReader(Logger *logger, RecordFileMap *file_map);
    int32_t Seek(UTime &stamp);
    int32_t ReadFrame(FRAME_INFO_T *frame_info);
    int32_t Close();

    void Shutdown();
};

}

#endif

