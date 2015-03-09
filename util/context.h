#ifndef STORAGE_CONTEXT_H_
#define STORAGE_CONTEXT_H_

namespace util
{

class Context
{
private:
    // no copying allowed
    Context(const Context& other);
    const Context& operator=(const Context& other);

    bool stop_;
protected:
    virtual void Finish(int r) = 0;

public:
    Context(bool stop = false) : stop_(stop) {}
    virtual ~Context() { }

    bool IsStopped() {return stop_;}
    
    virtual void Complete(int r)
    {
        Finish(r);
        delete this;
    }
};

}

#endif
