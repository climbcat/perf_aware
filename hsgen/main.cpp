#include "lib.h"


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



void CreateHaversinePointsJson(u32 npoints, u32 seed, bool use_sectors) {

    const char *filename_json = "hspairs.json";
    const char *filename_bin = "hsdist.bin";
    FILE *file_json = fopen(filename_json, "w");
    FILE *file_bin = fopen(filename_bin, "w");

    seed = RandInit(seed);
    f64 hsdist;
    f64 hsdist_sum = 0;
    f64 earth_radius = 6372.8;
    HsPair p;
    Sector s;

    u8 num_sectors_1d = 4;

    u32 sclen = num_sectors_1d + 1;
    f64 split_x[sclen];
    f64 split_y[sclen];
    split_x[0] = -180.0;
    split_x[sclen-1] = 180.0;
    split_y[0] = -90.0;
    split_y[sclen-1] = 90.0;
    for (int i = 1; i < sclen - 1; ++i) {
        split_x[i] = 180.0f * RandPM1();
        split_y[i] = 90.0f * RandPM1();
    }
    printf("split_x: %f %f %f %f %f\n", split_x[0], split_x[1], split_x[2], split_x[3], split_x[4]);
    printf("split_y: %f %f %f %f %f\n\n", split_y[0], split_y[1], split_y[2], split_y[3], split_y[4]);
    // sort split values
    f64 swap;

    u32 splitters = 5;
    for (int j = 0; j < sclen - 1; ++j) {
        for (int i = 0; i < sclen - 2 - j; ++i) {
            
            if (split_x[i] > split_x[i+1]) {
                swap = split_x[i];
                split_x[i] = split_x[i+1];
                split_x[i+1] = swap;
            }
            if (split_y[i] > split_y[i+1]) {
                swap = split_y[i];
                split_y[i] = split_y[i+1];
                split_y[i+1] = swap;
            }
        }
    }
    printf("split_x: %f %f %f %f %f\n", split_x[0], split_x[1], split_x[2], split_x[3], split_x[4]);
    printf("split_y: %f %f %f %f %f\n\n", split_y[0], split_y[1], split_y[2], split_y[3], split_y[4]);

    printf("sectors:\n");
    Sector sectors[sclen * sclen];
    for (int i = 0; i < sclen - 1; ++i) {
        for (int j = 0; j < sclen - 1; ++j) {
            s.xmin = split_x[i];
            s.xmax = split_x[i+1];
            s.ymin = split_y[j];
            s.ymax = split_y[j+1];

            sectors[num_sectors_1d*j + i] = s;
        }
    }
    for (int i = 0; i < sclen - 1; ++i) {
        for (int j = 0; j < sclen - 1; ++j) {
            PrintSector(sectors[num_sectors_1d*j + i]);
        }
    }
    exit(0);

    
    fprintf(file_json, "{\"pairs\":[\n");
    for (int i = 0; i < npoints; ++i) {

        // select sector
        if (use_sectors) {
            //xmin = sector_x[RandMinMaxI(0, 2)];
            //xmax = 
        }

        f64 xmin, xmax, ymin, ymax;
        // calculate point pair
        p = RandomPair(xmin, xmax, ymin, ymax);

        // calculate control answer
        hsdist = ReferenceHaversine(p.x0, p.y0, p.x1, p.y1, earth_radius);
        hsdist_sum += hsdist;
        
        // write pair to json
        fprintf(file_json, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}",
                p.x0, p.y0, p.x1, p.y1);
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
