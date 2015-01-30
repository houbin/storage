#ifndef STORAGE_H_
#define STORAGE_H_

#include "logger.h"

extern Logger *logger;
extern IdCenter *id_center;
extern StoreClientCenter *store_client_center;
extern FreeFileTable *free_file_table;
extern IndexFileManager *index_file_manager;

#endif

