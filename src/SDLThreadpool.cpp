#include "SDLThreadpool.h"

#include <EnergeticEngine/EEErrors.h>

SDLThreadpool::SDLThreadpool()
{
    maxThreads = SDL_GetCPUCount();
    pool = new SDL_Thread*[maxThreads];

	static const size_t bufSize = 32;
    for(int i = 0; i < maxThreads; ++i)
    {
        char name[bufSize];
        snprintf(name, bufSize, "pool%d", i);
        SDL_Thread* thread = SDL_CreateThread(poolFunc, name, this);
		if (NULL == thread) { // https://wiki.libsdl.org/SDL_GetThreadID
			ee::fatalError("SDL_CreateThread failed: " + std::string(SDL_GetError()) + "\n");
			//exit(-1);
		}
    }
}

SDLThreadpool::~SDLThreadpool()
{
    delete[] pool;
}

void SDLThreadpool::addJob(Updateable* job)
{
    queue.push(job);
}

void SDLThreadpool::closePool(void)
{
    //Event* death[maxThreads];
	Event** death = (Event**)alloca(sizeof(Event*) * maxThreads);

    for(int i = 0; i < maxThreads; ++i)
    {
        death[i] = new Event();
        queue.push(new Poison(death[i]));
    }

    for(int i = 0; i < maxThreads; ++i)
    {
        death[i]->wait();
		delete death[i];
    }
}

int SDLThreadpool::poolFunc(void* thisPointer)
{
    SDLThreadpool* tp = (SDLThreadpool*) thisPointer;
    tp->process();

    return 0;
}

void SDLThreadpool::process(void)
{
    while(status != Updateable::Stop)
    {
        Updateable* job = queue.pop();
        status = job->update();
		delete job;
    }
}
