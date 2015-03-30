#ifndef STORAGE_STORE_CLIENT_H_
#define STORAGE_STORE_CLIENT_H_

#include <deque>
#include "../include/errcode.h"
#include "../include/storage_api.h"
#include "../util/utime.h"
#include "../util/logger.h"
#include "../util/thread.h"
#include "../util/rwlock.h"
#include "store_types.h"
#include "free_file_table.h"
#include "record_writer.h"

using namespace std;
using namespace util;

namespace storage{

class StoreClient;
class RecordReader;
class RecordFile;

class RecordFileMap
{
private:
    Logger *logger_;

    StoreClient *store_client_;
    RWLock rwlock_;
    map<UTime, RecordFile*> record_file_map_;
    map<RecordFile*, map<UTime, RecordFile*>::iterator> file_search_map_;

    int32_t FindStampRecordFile(UTime &stamp, RecordFile **record_file);
    int32_t SelectFragInfoWithStartTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, deque<FRAGMENT_INFO_T> &select_frag_info);
    int32_t SelectFragInfoWithStartAndEndTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &select_frag_info);
    int32_t SelectFragInfoWithEndTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &end, deque<FRAGMENT_INFO_T> &select_frag_info);
    int32_t SelectAllFragInfo(deque<FRAGMENT_INFO_T> &all_frag_info, deque<FRAGMENT_INFO_T> &select_frag_info);

    void Dump();

public:
    RecordFileMap(Logger *logger, StoreClient *store_client);

    bool Empty();

    /* these functions just get or put record file */
    int32_t PutRecordFile(UTime &time, RecordFile *record_file);
    int32_t ListRecordFragments(UTime &time, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue);
    int32_t GetLastRecordFile(RecordFile **record_file);

    /* these functions change states of record file */
    int32_t OpenWriteRecordFile(RecordFile **record_file);
    int32_t AllocWriteRecordFile(UTime &stamp, RecordFile **record_file);
    int32_t FinishWriteRecordFile(RecordFile *record_file);
    int32_t EraseRecordFile(RecordFile *record_file);
    int32_t SeekStampOffset(UTime &stamp, RecordFile **record_file, uint32_t &seek_start_offset, uint32_t &seek_end_offset);
    int32_t FinishReadRecordFile(RecordFile *record_file);

    void Shutdown();
};

class StoreClient
{
private:
    Logger *logger_;

    string stream_info_;
    RecordFileMap record_file_map_;

    RecordWriter writer;

    Mutex reader_mutex_;
    map<int32_t, RecordReader*> record_readers_;

public:
    StoreClient(Logger *logger, string stream_info);

    bool IsRecordFileEmpty();
    string GetStreamInfo();

    int32_t OpenWrite(int32_t id);
    int32_t EnqueueFrame(FRAME_INFO_T *frame);
    int32_t CloseWrite(int32_t id);

    int32_t OpenRead(int32_t id);
    int32_t SeekRead(int32_t id, UTime &stamp);
    int32_t ReadFrame(int32_t id, FRAME_INFO_T *frame);
    int32_t CloseRead(int32_t id);

    int32_t GetFreeFile(RecordFile **record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);
    int32_t SeekReaderStampOffset(UTime &stamp, RecordFile **record_file, uint32_t &seek_start_offset, uint32_t &seek_end_offset);
    int32_t PutRecordFile(UTime &stamp, RecordFile *record_file);
    int32_t RecycleRecordFile(RecordFile *record_file);

    int32_t WriteRecordFileIndex(RecordFile *record_file, int r);

    int32_t ListRecordFragments(UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue);

    void Shutdown();
};

}

#endif

