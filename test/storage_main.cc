#include <stdlib.h>
#include <assert.h>
#include "test_writer.h"
#include "test_reader.h"
#include "../util/config.h"
#include "storage_main.h"
#include "../include/storage.h"

using namespace std;
using namespace util;

int main()
{
    Config *config = new Config("./config.txt");

    string write_thread_count_str;
    string read_thread_count_str;
    write_thread_count_str = config->Read("frame_write_thread_count", write_thread_count_str);
    read_thread_count_str = config->Read("frame_read_thread_count", read_thread_count_str);

    int write_thread_count = atoi(write_thread_count_str.c_str());
    int read_thread_count = atoi(read_thread_count_str.c_str());

    fprintf(stderr, "write thread count is %d, write_thread_count_str is %s\n", write_thread_count, write_thread_count_str.c_str());
    fprintf(stderr, "read thread count is %d, read_thread_count_str is %s\n", read_thread_count, read_thread_count_str.c_str());

    storage_init();

    store_client_center->DumpClientSearchMap();

    TestThread write_threads("writer", write_thread_count);
    TestThread read_threads("reader", read_thread_count);

    write_threads.Start();
    read_threads.Start();

    write_threads.Shutdown();
    read_threads.Shutdown();

    storage_shutdown();

    return 0;
}
