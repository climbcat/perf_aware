#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <assert.h>
#include <cstdint>
#include "math.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef double f64;

#define MEGABYTE 1024*1024
#define GIGABYTE 1024*1024*1024


//
// cmd line args


bool ContainsArg(const char *search, int argc, char **argv, int *idx = NULL) {
    for (int i = 0; i < argc; ++i) {
        char *arg = argv[i];
        if (!strcmp(argv[i], search)) {
            if (idx != NULL) {
                *idx = i;
            }
            return true;
        }
    }
    return false;
}


//
// file I/O


char *LoadFileMMAP(char *filepath, u64 *size_bytes = NULL) {
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        printf("Could not open file: %s\n", filepath);
        exit(1);
    }

    s32 fd = fileno(f);
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        printf("Could not get file size: %s\n", filepath);
        exit(1);
    }

    char *str = (char*) mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE | MAP_SHARED, fd, 0);
    if (size_bytes != NULL) {
        *size_bytes = sb.st_size;
    }

    fclose(f);
    return str;
}


//
// timing


u64 GetSystimeMySec() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // microsecs 

    return systime;
}

u64 GetRdtsc() {
    u64 ticks;
    unsigned c,d;
    asm volatile("rdtsc" : "=a" (c), "=d" (d));
    ticks = (( (u64)c ) | (( (u64)d ) << 32)); // unknown cpu units

    return ticks;
}

void CalibrateRdtsc(u64 num_my_secs) {


    u64 ticks_start = GetRdtsc();
    u64 systime_start = GetSystimeMySec();

    // busy wait
    while (GetSystimeMySec() - systime_start < num_my_secs) {}

    u64 ticks_diff = GetRdtsc() - ticks_start;
    u64 systime_diff = GetSystimeMySec() - systime_start;

    float units = ticks_diff / (float) systime_diff;

    printf("Processor Frequency [MHz] (rdtsc pr. mysec over %.f millisecs): %f\n", num_my_secs / (float) 1000, units);
}


//
// Haversine functions


static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 Deg2Rad(f64 Degrees)
{
    f64 Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = Deg2Rad(lat2 - lat1);
    f64 dLon = Deg2Rad(lon2 - lon1);
    lat1 = Deg2Rad(lat1);
    lat2 = Deg2Rad(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}
