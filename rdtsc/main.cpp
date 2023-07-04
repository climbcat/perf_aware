#include "../lib.c"


void CalibrateRdtsc(u64 num_my_secs) {
    u64 ticks_start = ReadCPUTimer();
    u64 systime_start = ReadSystemTimerMySec();

    // busy wait
    while (ReadSystemTimerMySec() - systime_start < num_my_secs) {}

    u64 ticks_diff = ReadCPUTimer() - ticks_start;
    u64 systime_diff = ReadSystemTimerMySec() - systime_start;

    float units = ticks_diff / (float) systime_diff;

    printf("Processor Frequency [MHz] (rdtsc pr. mysec over %.f millisecs): %f\n", num_my_secs / (float) 1000, units);
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
