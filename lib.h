#ifndef __LIB_H__
#define __LIB_H__

#include <sys/time.h>
#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define KILOBYTE 1024
#define MEGABYTE 1024 * 1024
#define SIXTEEN_M 16 * 1024 * 1024

#define SIXTEEN_K 16 * 1024
#define THIRTYTWO_K 32 * 1024
#define SIXTYFOUR_K 64 * 1024


// RANDOM

#ifndef ULONG_MAX
#  define ULONG_MAX ((unsigned long)0xffffffffffffffffUL)
#endif


void Kiss_SRandom(unsigned long state[7], unsigned long seed) {
    if (seed == 0) seed = 1;
    state[0] = seed | 1; // x
    state[1] = seed | 2; // y
    state[2] = seed | 4; // z
    state[3] = seed | 8; // w
    state[4] = 0;        // carry
}
unsigned long Kiss_Random(unsigned long state[7]) {
    state[0] = state[0] * 69069 + 1;
    state[1] ^= state[1] << 13;
    state[1] ^= state[1] >> 17;
    state[1] ^= state[1] << 5;
    state[5] = (state[2] >> 2) + (state[3] >> 3) + (state[4] >> 2);
    state[6] = state[3] + state[3] + state[2] + state[4];
    state[2] = state[3];
    state[3] = state[6];
    state[4] = state[5] >> 30;
    return state[0] + state[1] + state[3];
}
unsigned long _hash(unsigned long x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
unsigned long g_state[7];
bool g_didinit = false;
#define Random() Kiss_Random(g_state)
void RandInit() {
    if (g_didinit == true)
        return;

    struct timeval tm;
    gettimeofday(&tm, NULL);
    unsigned long seed = _hash((unsigned long) tm.tv_sec*1000000 + tm.tv_usec);
    Kiss_SRandom(g_state, seed);

    g_didinit = true;
}

double Rand01() {
    double randnum;
    randnum = (double) Random();
    randnum /= (double) ULONG_MAX + 1;
    return randnum;
}
double RandPM1() {
    double randnum;
    randnum = (double) Random();
    randnum /= ((double) ULONG_MAX + 1) / 2;
    randnum -= 1;
    return randnum;
}

#endif