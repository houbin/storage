#ifndef STORAGE_MAIN_H_
#define STORAGE_MAIN_H_

void StartWrite(int write_thread_count)
{
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

    return;
}

void StartRead(int read_thread_count)
{
    FrameReader **read_array = new FrameReader*[read_thread_count];
    assert(read_array != NULL);
    for (int i = 0; i < read_thread_count; i++)
    {
        read_array[i] = new FrameReader(i);
        assert(read_array[i] != NULL);
    }

    for (int i = 0; i < read_thread_count; i++)
    {
        read_array[i]->Start();
    }

    for (int i = 0; i < read_thread_count; i++)
    {
        read_array[i]->Shutdown();
    }

    for (int i = 0; i < read_thread_count; i++)
    {
        delete read_array[i];
    }

    delete []read_array;
    return;
}

class TestThread : public Thread
{
private:
    string name_;
    int thread_count_;
    
public:
    TestThread(string name, int thread_count) : name_(name), thread_count_(thread_count) {}
    
    int Start()
    {
        Create();

        return 0;
    }

    void *Entry()
    {
        if (name_ == "writer")
        {
            StartWrite(thread_count_);
        }
        else
        {
            StartRead(thread_count_);
        }

        return NULL;
    }

    void Shutdown()
    {
        Join();
    }
};

#endif
