#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>


typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

bool CLAContainsArg(const char *search, int argc, char **argv, int *idx = NULL) {
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
u32 ParseInt(char *text) {
    u32 val = 0;
    u32 multiplier = 1;

    // signed?
    bool sgned = text[0] == '-';
    if (sgned) {
        ++text;
    }

    u32 len = strlen(text);

    // decimals before dot
    for (int i = 0; i < len; ++i) {
        val += (text[len - 1 - i] - 48) * multiplier;
        multiplier *= 10;
    }

    // handle the sign
    if (sgned) {
        val *= -1;
    }

    return val;
}
