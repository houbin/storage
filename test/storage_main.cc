#include <stdlib.h>
#include <assert.h>
#include "storage_test.h"
#include "../util/config.h"

using namespace std;
using namespace util;

int main()
{
    Config *config = new Config("./config.txt");
    string write_thread_count_str;
    write_thread_count_str = config->Read("frame_write_thread_count", write_thread_count_str);
    int write_thread_count = atoi(write_thread_count_str.c_str());

    fprintf(stderr, "write thread count is %d, write_thread_count_str is %s\n", write_thread_count, write_thread_count_str.c_str());

    storage_init();

    FrameWriter **write_array = new FrameWriter*[write_thread_count];
    assert(write_array != NULL);
    for (int i = 0; i < write_thread_count; i++)
    {
        write_array[i] = new FrameWriter(i);
        assert(write_array[i] != NULL);
    }

    for (int i = 0; i < write_thread_count; i++)
    {
        write_array[i]->Start();
    }

    for (int i = 0; i < write_thread_count; i++)
    {
        write_array[i]->Shutdown();
    }

    for (int i = 0; i < write_thread_count; i++)
    {
        delete write_array[i];
    }

    delete []write_array;

    storage_shutdown();
    return 0;
}
