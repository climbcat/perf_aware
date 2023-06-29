#include "../lib.c"


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
