#include "JackieWaitEvent.h"
#include "geco-time.h"

GECO_NET_BEGIN_NSPACE
JackieWaitEvent::JackieWaitEvent()
{
#ifdef _WIN32
    eventList = INVALID_HANDLE_VALUE;
#else
    isSignaled = false;
#endif
}


JackieWaitEvent::~JackieWaitEvent()
{
    // Intentionally do not close event, so it doesn't close twice on linux
}

void JackieWaitEvent::Close(void)
{
#ifdef _WIN32
    if (eventList != INVALID_HANDLE_VALUE)
    {
        CloseHandle(eventList);
        eventList = INVALID_HANDLE_VALUE;
    }
#else
    pthread_cond_destroy(&eventList);
    pthread_mutex_destroy(&hMutex);
#if !defined(ANDROID)
    pthread_condattr_destroy(&condAttr);
#endif
    pthread_mutexattr_destroy(&mutexAttr);
#endif
}

void JackieWaitEvent::TriggerEvent(void)
{
#ifdef _WIN32
    SetEvent(eventList);
#else
    // Different from SetEvent in windows which stays signaled.
    // We have to record manually that the event was signaled
    isSignaledMutex.Lock();
    isSignaled=true;
    isSignaledMutex.Unlock();
    // Unblock waiting threads
    pthread_cond_broadcast(&eventList);
#endif
}

void JackieWaitEvent::WaitEvent(int timeoutMs)
{
#ifdef _WIN32
    WaitForSingleObjectEx(eventList, timeoutMs, FALSE);
#else

    // If was previously set signaled, just unset and return
    isSignaledMutex.Lock();
    if (isSignaled==true)
    {
        isSignaled = false;
        isSignaledMutex.Unlock();
        return;
    }
    isSignaledMutex.Unlock();

    //struct timespec   ts
    // Else wait for SetEvent to be called
    struct timespec   ts;

    int rc;
    timeval tp;
    rc = gettimeofday(&tp, 0);
    ts.tv_sec = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    // #endif

    while( timeoutMs > 30 )
    {
        // Wait 30 milliseconds for the signal, then check again.
        // This is in case we  missed the signal between the top of this function and pthread_cond_timedwait, or after the end of the loop and pthread_cond_timedwait
        ts.tv_nsec += 30 * 1000000;
        if( ts.tv_nsec >= 1000000000 )
        {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec++;
        }

        // [SBC] added mutex lock/unlock around cond_timedwait.
        // this prevents airplay from generating a whole much of errors.
        // not sure how this works on other platforms since according to
        // the docs you are suppost to hold the lock before you wait
        // on the cond.
        pthread_mutex_lock(&hMutex);
        pthread_cond_timedwait(&eventList, &hMutex, &ts);
        pthread_mutex_unlock(&hMutex);

        timeoutMs -= 30;

        isSignaledMutex.Lock();
        if( isSignaled == true )
        {
            isSignaled = false;
            isSignaledMutex.Unlock();
            return;
        }
        isSignaledMutex.Unlock();
    }

    // Wait the remaining time, and turn off the signal in case it was set
    ts.tv_nsec += timeoutMs * 1000000;
    if( ts.tv_nsec >= 1000000000 )
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }

    pthread_mutex_lock(&hMutex);
    pthread_cond_timedwait(&eventList, &hMutex, &ts);
    pthread_mutex_unlock(&hMutex);

    isSignaledMutex.Lock();
    isSignaled = false;
    isSignaledMutex.Unlock();

#endif
}

void JackieWaitEvent::Init(void)
{
#if defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
    eventList = CreateEventEx(0, 0, 0, 0);
#elif defined(_WIN32)
    eventList = CreateEvent(0, false, false, 0);
#else
#if !defined(ANDROID)
    pthread_condattr_init(&condAttr);
    pthread_cond_init(&eventList, &condAttr);
#else
    pthread_cond_init(&eventList, 0);
#endif
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutex_init(&hMutex, &mutexAttr);
#endif
}
GECO_NET_END_NSPACE
