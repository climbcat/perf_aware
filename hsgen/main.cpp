#include "lib.h"


struct HsPair {
    f64 x0 = 0;
    f64 y0 = 0;
    f64 x1 = 0;
    f64 y1 = 0;
};

HsPair RandomPair(f64 xmin, f64 xmax, f64 ymin, f64 ymax) {
    HsPair p;

    p.x0 = (xmax - xmin) * RandPM1() + xmin;
    p.y0 = (ymax - ymin) * RandPM1() + ymin;
    p.x1 = (xmax - xmin) * RandPM1() + xmin;
    p.y1 = (ymax - ymin) * RandPM1() + ymin;

    return p;
}

void CreateHaversinePointsJson(u32 npoints, u32 seed, bool sectors) {

    const char *filename_json = "hspairs.json";
    const char *filename_bin = "hsdist.bin";
    FILE *file_json = fopen(filename_json, "w");
    FILE *file_bin = fopen(filename_bin, "w");

    seed = RandInit(seed);
    f64 x0, y0, x1, y1, hsdist, hsdist_sum = 0, earth_radius = 6372.8;

    f64 sector_x[3];
    f64 sector_y[3];
    f64 sector_sum[3*3];
    for (int i = 0; i < 4; ++i) {
        sector_x[i] = 180.0f * RandPM1();
        sector_y[i] = 90.0f * RandPM1();
    }
    
    fprintf(file_json, "{\"pairs\":[\n");
    for (int i = 0; i < npoints; ++i) {

        // TODO: select a sector (two random ints from 0-3 to hit x, y sector)

        // calculate point pair
        HsPair p = RandomPair(-180.0, 180.0, -90.0, 90.0);
        x0 = p.x0;
        y0 = p.y0;
        x1 = p.x1;
        y1 = p.y1;

        // calculate control answer
        hsdist = ReferenceHaversine(x0, y0, x1, y1, earth_radius);
        hsdist_sum += hsdist;
        
        // write pair to json
        fprintf(file_json, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}",
                x0, y0, x1, y1);
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

int main (int argc, char **argv) {
    if (ContainsArg("--help", argc, argv) || ContainsArg("-h", argc, argv)) {
        printf("--help:          display help (this text)\n--test:          run tests and exit\n--sectors:       fragment points into sectors\n--npoits [int]:  number of points to generate\n--seed [int]:    set random seed\n");
        exit(0);
    }
    if (ContainsArg("--test", argc, argv)) {
        Test();
        exit(0);
    }

    bool sectors = false;
    if (ContainsArg("--sectors", argc, argv)) {
        sectors = true;
    }
    u32 seed = 0;
    if (ContainsArg("--seed", argc, argv)) {
        seed = ParseInt(GetArgValue("--seed", argc, argv));
    }
    u32 npoints = 20;
    if (ContainsArg("--npoints", argc, argv)) {
        npoints = ParseInt(GetArgValue("--npoints", argc, argv));
    }

    CreateHaversinePointsJson(npoints, seed, sectors);
}
