#ifndef _THREADSAFEQUEUE_H_
#define _THREADSAFEQUEUE_H_

#include <queue>
#include <SDL2/SDL.h>

template <class T>
class ThreadSafeQueue
{
    private:
        std::queue<T> workQueue;
        SDL_mutex* lock;
        SDL_cond* available;

    public:
        ThreadSafeQueue(void);
        ~ThreadSafeQueue(void);

        ThreadSafeQueue(ThreadSafeQueue* q);

        void push(T job);
        T pop(void);
};

template <class T>
ThreadSafeQueue<T>::ThreadSafeQueue(void)
{
    lock = SDL_CreateMutex();
    available = SDL_CreateCond();
}

template <class T>
ThreadSafeQueue<T>::~ThreadSafeQueue(void)
{
    SDL_LockMutex(lock);
    while(!workQueue.empty())
        workQueue.pop();
    SDL_UnlockMutex(lock);

    SDL_DestroyMutex(lock);
    SDL_DestroyCond(available);
}

//ThreadSafeQueue::ThreadSafeQueue(ThreadSafeQueue* q);

template <class T>
void ThreadSafeQueue<T>::push(T job)
{
    SDL_LockMutex(lock);
    workQueue.push(job);
    SDL_CondBroadcast(available);
    SDL_UnlockMutex(lock);
}

template <class T>
T ThreadSafeQueue<T>::pop(void)
{
    SDL_LockMutex(lock);
    while(workQueue.empty())
        SDL_CondWait(available,lock);
    T top = workQueue.front();
    workQueue.pop();
    SDL_UnlockMutex(lock);

    return top;
}

#endif
