#ifndef STORAGE_H_
#define STORAGE_H_

#include "../util/logger.h"
#include "id_center.h"
#include "store_client_center.h"
#include "free_file_table.h"
#include "index_file.h"
#include "bad_disks.h"

using namespace storage;

extern Logger *logger;
extern IdCenter *id_center;
extern StoreClientCenter *store_client_center;
extern FreeFileTable *free_file_table;
extern IndexFileManager *index_file_manager;
extern RecordRecycle *record_recycle;
extern BadDiskMap *bad_disk_map;

#endif

