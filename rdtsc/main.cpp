#include "lib.c"

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
    printf("Calibrating rdtsc pr. ms over %lu microsecs / %.3f millisecs \n", num_my_secs, num_my_secs / (float) 1000);

    u64 ticks_start = GetRdtsc();
    u64 systime_start = GetSystimeMySec();

    // busy wait
    while (GetSystimeMySec() - systime_start < num_my_secs) {}

    u64 ticks_diff = GetRdtsc() - ticks_start;
    u64 systime_diff = GetSystimeMySec() - systime_start;

    float units = ticks_diff / (float) systime_diff;
    printf("rdtsc pr. mysec: %f\n", units);
}




int main (int argc, char **argv) {
    if (CLAContainsArg("--help", argc, argv)) {
        printf("Usage:\n        timer <interval_mysecs>\n");
        exit(0);
    }

    u64 interval_mysecs;
    if (argc == 1) {
        interval_mysecs = 10000; // 10 millisecs gives a rough estimate
    }
    else {
        interval_mysecs = ParseInt(argv[1]);
    }
    CalibrateRdtsc(interval_mysecs);
}
