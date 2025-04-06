#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <stddef.h>

int get_ms() {
    #if defined(_WIN32) || defined(_WIN64)
        return GetTickCount();
    #else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    #endif
}
