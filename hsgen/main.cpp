#include "lib.h"

void CreateHaversinePointsJson(u32 num_points, u32 seed) {

    FILE *file_json = fopen("hspairs.json", "w");
    FILE *file_bin = fopen("hsdist.bin", "w");

    seed = RandInit(seed);
    f64 x0, y0, x1, y1, hsdist, hsdist_sum = 0, earth_radius = 6372.8;

    // TODO: do sector setup here (16 square sectors, 4 x and 4 y sectors, 3 randon x and y values as separators)

    fprintf(file_json, "{\"pairs\":[\n");
    for (int i = 0; i < num_points; ++i) {

        // TODO: select a sector (two random ints from 0-3 to hit x, y sector)

        // calculate point pair
        x0 = 180.0f * RandPM1();
        y0 =  90.0f * RandPM1();
        x1 = 180.0f * RandPM1();
        y1 =  90.0f * RandPM1();

        // calculate control answer
        hsdist = ReferenceHaversine(x0, y0, x1, y1, earth_radius);
        hsdist_sum += hsdist;

        // write pair to json
        fprintf(file_json, "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}", x0, y0, x1, y1);
        if (i + 1 < num_points) {
            fprintf(file_json, ",");
        }
        fprintf(file_json, "\n");

        // write answer to binary
        fwrite(&hsdist, 8, 1, file_bin);
    }
    fprintf(file_json, "]}\n");

    fclose(file_json);
    fclose(file_bin);

    printf("Haversine distance data generated from seed=%d:\n    sum = %.16f\n    average = %.16f\n", seed, hsdist_sum, hsdist_sum / num_points);
}

void RunTests() {
    printf("running tests ...\n");
    printf("done\n");
}

int main (int argc, char **argv) {
    // TODO: cmd line args:
    //      --test

    //RunTests();

    // TODO: cmd line args:
    //      --sectors
    //      --seed=value where zero means generate new
    //      --npoints=value

    u32 npoints = 20;
    u32 seed = 0;
    CreateHaversinePointsJson(npoints, seed);
}
