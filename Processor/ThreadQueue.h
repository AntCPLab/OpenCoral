/*
 * ThreadQueue.h
 *
 */

#ifndef PROCESSOR_THREADQUEUE_H_
#define PROCESSOR_THREADQUEUE_H_

class ThreadQueue
{
    WaitQueue<ThreadJob> in, out;
    Lock lock;
    int left;

public:
    ThreadQueue() :
            left(0)
    {
    }

    bool available()
    {
        return left == 0;
    }

    void schedule(ThreadJob job)
    {
        lock.lock();
        left++;
#ifdef DEBUG_THREAD_QUEUE
        cerr << this << ": " << left << " left" << endl;
#endif
        lock.unlock();
        in.push(job);
    }

    ThreadJob next()
    {
        return in.pop();
    }

    void finished(ThreadJob job)
    {
        out.push(job);
    }

    ThreadJob result()
    {
        auto res = out.pop();
        lock.lock();
        left--;
#ifdef DEBUG_THREAD_QUEUE
        cerr << this << ": " << left << " left" << endl;
#endif
        lock.unlock();
        return res;
    }
};

#endif /* PROCESSOR_THREADQUEUE_H_ */
