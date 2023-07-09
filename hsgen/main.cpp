#include <cstdlib>
#include <cstdio>

#include <cstdint>
#include <cassert>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "../../baselayer/base.c"
#include "../../baselayer/memory.c"
#include "../../baselayer/utils.c"
#include "../../baselayer/string.c"
#include "../../baselayer/profile.c"

#include "../haversine.c"


struct HsPair {
    f64 x0 = 0;
    f64 y0 = 0;
    f64 x1 = 0;
    f64 y1 = 0;
};

struct Sector {
    f64 xmin = -180.0;
    f64 xmax = 180.0;
    f64 ymin = -90.0;
    f64 ymax = 90.0;
    f64 sum = 0;
};

HsPair RandomPair(f64 xmin, f64 xmax, f64 ymin, f64 ymax) {
    HsPair p;

    p.x0 = (xmax - xmin) * RandPM1() + xmin;
    p.y0 = (ymax - ymin) * RandPM1() + ymin;
    p.x1 = (xmax - xmin) * RandPM1() + xmin;
    p.y1 = (ymax - ymin) * RandPM1() + ymin;

    return p;
}

void PrintSector(Sector s) {
    printf("%f %f %f %f\n", s.xmin, s.xmax, s.ymin, s.ymax);
}



void CreateHaversinePointsJson(u32 npoints, u32 seed) {

    const char *filename_json = "hspairs.json";
    const char *filename_bin = "hsdist.bin";
    FILE *file_json = fopen(filename_json, "w");
    FILE *file_bin = fopen(filename_bin, "w");

    seed = RandInit(seed);
    f64 hsdist;
    f64 hsdist_sum = 0;
    f64 earth_radius = 6372.8;
    HsPair p;

    fprintf(file_json, "{\"pairs\":[\n");

    Sector s;
    for (int i = 0; i < npoints; ++i) {
        // calculate point pair
        p = RandomPair(s.xmin, s.xmax, s.ymin, s.ymax);

        // calculate control answer
        hsdist = ReferenceHaversine(p.x0, p.y0, p.x1, p.y1, earth_radius);
        hsdist_sum += hsdist;
        
        // write pair to json
        fprintf(file_json, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}", p.x0, p.y0, p.x1, p.y1);
        if (i + 1 < npoints) {
            fprintf(file_json, ",");
        }
        fprintf(file_json, "\n");

        // write answer to binary
        fwrite(&hsdist, 8, 1, file_bin);
    }
    fprintf(file_json, "]}\n");

    fclose(file_json);
    fclose(file_bin);

    printf("Haversine distance data generated using\n    seed:    %u\n    npoints: %u\n",
           seed, npoints);
    printf("Yeilding\n    sum:     %.16f\n    average: %.16f\n",
           hsdist_sum, hsdist_sum / npoints);
    printf("Pairs data file: %s\n", filename_json);
    printf("Haversine answers file: %s\n", filename_bin);    
}

void Test() {
    printf("running tests ...\n");
    printf("done\n");
}

int main (int argc, char **argv) {
    if (CLAContainsArg("--help", argc, argv) || CLAContainsArg("-h", argc, argv)) {
        printf("\
    Usage:\n\n\
        hsgen --npoints 10000\n\
        hsgen --npoints 10000 -seed 0\n\n\
        --help:          display help (this text)\n \
        --test:          run tests and exit\n\
        --npoits [int]:  number of points to generate\n\
        --seed [int]:    set random seed\n");
        exit(0);
    }
    if (CLAContainsArg("--test", argc, argv)) {
        Test();
        exit(0);
    }

    u32 seed = 0;
    if (CLAContainsArg("--seed", argc, argv)) {
        seed = ParseInt(CLAGetArgValue("--seed", argc, argv));
    }
    u32 npoints = 20;
    if (CLAContainsArg("--npoints", argc, argv)) {
        npoints = ParseInt(CLAGetArgValue("--npoints", argc, argv));
    }

    CreateHaversinePointsJson(npoints, seed);
}
