#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "lib.h"

void WritePointToFile(FILE *file, double theta, double phi, double answer) {
    fprintf(file, "[%.16f, %.16f, %.16f]", theta, phi, answer);
}

double HaversineDistance(double theta, double phi) {
    return 0.0;
}

void CreateHaversinePointsJson(u32 num_points) {

    const char* filename = "haversine.json";
    FILE *file = fopen(filename, "w");

    RandInit();
    double theta, phi, hsdist, hsdist_sum = 0;

    fprintf(file, "{\"points\" : \n");
    for (int i = 0; i < num_points; ++i) {
        // calculate
        theta = 180.0f * Rand01();
        phi = 90.0f * RandPM1();
        hsdist = HaversineDistance(theta, phi);
        hsdist_sum += hsdist;

        // write to file
        fprintf(file, "    ");
        WritePointToFile(file, theta, phi, hsdist);
        if (i + 1 < num_points) {

            fprintf(file, ",");
        }
        fprintf(file, "\n");
    }
    fprintf(file, "}\n");

    fclose(file);
}

void RunTests() {
    printf("running tests ...\n");
    printf("done\n");
}

int main (int argc, char **argv) {
    //RunTests();

    CreateHaversinePointsJson(20);

}
