#include <assert.h>
#include "storage_test.h"

#define FRAME_WRITE_THREAD_COUNT 1

int main()
{
    storage_init();

    FrameWriter **write_array = new FrameWriter*[FRAME_WRITE_THREAD_COUNT];
    assert(write_array != NULL);
    for (int i = 0; i < FRAME_WRITE_THREAD_COUNT; i++)
    {
        write_array[i] = new FrameWriter(i);
        assert(write_array[i] != NULL);
    }

    for (int i = 0; i < FRAME_WRITE_THREAD_COUNT; i++)
    {
        write_array[i]->Start();
    }

    for (int i = 0; i < FRAME_WRITE_THREAD_COUNT; i++)
    {
        write_array[i]->Shutdown();
    }

    for (int i = 0; i < FRAME_WRITE_THREAD_COUNT; i++)
    {
        delete write_array[i];
    }

    delete []write_array;

    storage_shutdown();
    return 0;
}
