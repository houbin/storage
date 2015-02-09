#ifndef STORAGE_H_
#define STORAGE_H_

#include "logger.h"
#include "../src/id_center.h"
#include "../src/store_client_center.h"
#include "../src/free_file_table.h"
#include "../src/index_file.h"

using namespace storage;

extern Logger *logger;
extern IdCenter *id_center;
extern StoreClientCenter *store_client_center;
extern FreeFileTable *free_file_table;
extern IndexFileManager *index_file_manager;

#endif

