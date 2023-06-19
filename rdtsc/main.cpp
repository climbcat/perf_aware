#include "lib.c"

u64 GetSystime_Ms() {
    u64 systime;
    struct timeval tm;
    gettimeofday(&tm, NULL);
    systime = (u32) tm.tv_sec*1000000 + tm.tv_usec; // millisecs

    return systime;
}

u64 GetRdtsc() {
    u64 ticks;
    unsigned c,d;
    asm volatile("rdtsc" : "=a" (c), "=d" (d));
    ticks = (( (u64)c ) | (( (u64)d ) << 32)); // unknown cpu units

    return ticks;
}

void CalibrateRdtsc(u32 num_secs) {

    printf("Calibrating rdtsc pr. ms over %u secs...\n", num_secs);

    u64 ticks_start = GetRdtsc();
    u64 systime_start = GetSystime_Ms();
    sleep(num_secs);
    u64 ticks_diff = GetRdtsc() - ticks_start;
    u64 systime_diff = GetSystime_Ms() - systime_start;

    //printf("cpu (tsc): %lu, sys (ms): %lu\n", ticks_diff, systime_diff);
    f64 units = ticks_diff / (f64) systime_diff;
    printf("rdtsc pr. ms: %f\n", units);
}




int main (int argc, char **argv) {
    if (CLAContainsArg("--help", argc, argv) || argc != 2) {
        printf("Usage:\n        timer <calibration_secs>\n");
        exit(0);
    }

    u32 interval_secs = ParseInt(argv[1]);
    CalibrateRdtsc(interval_secs);
}
