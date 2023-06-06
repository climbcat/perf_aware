#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "lib.h"
#include "math.h"

typedef double f64;
typedef uint32_t u32;


static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 Deg2Rad(f64 Degrees)
{
    f64 Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */

    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = Deg2Rad(lat2 - lat1);
    f64 dLon = Deg2Rad(lon2 - lon1);
    lat1 = Deg2Rad(lat1);
    lat2 = Deg2Rad(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}

void CreateHaversinePointsJson(u32 num_points, u32 seed) {

    FILE *file_json = fopen("hspairs.json", "w");
    FILE *file_bin = fopen("hsdist_answers.bin", "w");

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

        // DEBUG: 
        //printf("%.16f\n", hsdist);

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
