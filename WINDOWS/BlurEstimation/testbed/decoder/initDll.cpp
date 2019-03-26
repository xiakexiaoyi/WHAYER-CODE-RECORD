
//use ffmpeg
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
};


#ifdef _MSC_VER

#ifdef _WIN32

#pragma comment(lib, "libavcodec.a")
#pragma comment(lib, "libavutil.a")
#pragma comment(lib, "libswscale.a")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libx264.lib")
//#pragma comment(lib, "vfw32.lib")



//#pragma comment(lib, "avcodec.lib")
//#pragma comment(lib, "avutil.lib")
//#pragma comment(lib, "swscale.lib")


#endif


#endif//_MSC_VER



class ImplMutex
{
public:
    ImplMutex() { init(); }
    ~ImplMutex() { destroy(); }

    void init();
    void destroy();

    void lock();
    bool trylock();
    void unlock();

    struct Impl;
protected:
    Impl* impl;

private:
    ImplMutex(const ImplMutex&);
    ImplMutex& operator = (const ImplMutex& m);
};

#if defined WIN32 || defined _WIN32 || defined WINCE
#include <windows.h>
struct ImplMutex::Impl
{
    void init() { InitializeCriticalSection(&cs); refcount = 1; }
    void destroy() { DeleteCriticalSection(&cs); }

    void lock() { EnterCriticalSection(&cs); }
    bool trylock() { return TryEnterCriticalSection(&cs) != 0; }
    void unlock() { LeaveCriticalSection(&cs); }

    CRITICAL_SECTION cs;
    int refcount;
};
#else
#include <pthread.h>
struct ImplMutex::Impl
{
    void init() { pthread_spin_init(&sl, 0); refcount = 1; }
    void destroy() { pthread_spin_destroy(&sl); }

    void lock() { pthread_spin_lock(&sl); }
    bool trylock() { return pthread_spin_trylock(&sl) == 0; }
    void unlock() { pthread_spin_unlock(&sl); }

    pthread_spinlock_t sl;
    int refcount;
};
#endif

void ImplMutex::init()
{
    impl = (Impl*)malloc(sizeof(Impl));
    impl->init();
}
void ImplMutex::destroy() 
{
    impl->destroy();
    free(impl);
    impl = NULL;
}
void ImplMutex::lock() { impl->lock(); }
void ImplMutex::unlock() { impl->unlock(); }
bool ImplMutex::trylock() { return impl->trylock(); }

static int LockCallBack(void **mutex, AVLockOp op)
{
    ImplMutex* localMutex = reinterpret_cast<ImplMutex*>(*mutex);
    switch (op)
    {
    case AV_LOCK_CREATE:
        localMutex = reinterpret_cast<ImplMutex*>(malloc(sizeof(ImplMutex)));
        localMutex->init();
        *mutex = localMutex;
        if (!*mutex)
            return 1;
        break;

    case AV_LOCK_OBTAIN:
        localMutex->lock();
        break;

    case AV_LOCK_RELEASE:
        localMutex->unlock();
        break;

    case AV_LOCK_DESTROY:
        localMutex->destroy();
        free(localMutex);
        localMutex = NULL;
        break;
    }
    return 0;
}

static ImplMutex _mutex;
static bool _initialized = false;


//初始化
void initHyPlayLibrary()
{
    _mutex.lock();
    if (!_initialized)
    {
        avcodec_register_all();

        /* register a callback function for synchronization */
        av_lockmgr_register(&LockCallBack);
        av_log_set_level(AV_LOG_ERROR);
        _initialized = true;
    }
    _mutex.unlock();
}

//反初始化
void uninitHyPlayLibrary()
{
    _initialized = false;
    av_lockmgr_register(NULL);
}

#ifdef _WIN32
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
//BOOL WINAPI _CRT_INIT (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)   // sunqueen modify
{
    (void) hinstDll;
    (void) lpvReserved;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        initHyPlayLibrary();
        break;

    case DLL_PROCESS_DETACH:
        uninitHyPlayLibrary();
        break;
    }
    return TRUE;
}
#else
struct InitFini
{
    InitFini()
    {
        initHyPlayLibrary();
    }
    ~InitFini()
    {
        uninitHyPlayLibrary();
    }
};
static InitFini _initfini;
#endif
