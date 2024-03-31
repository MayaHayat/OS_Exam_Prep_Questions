#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define CANVAS_SIZE 100

int point_in_ellipse(double x, double y, double a, double b) {
    return ((x - 50) * (x - 50)) / (a * a) + ((y - 50) * (y - 50)) / (b * b) <= 1;
}

double monte_carlo_ellipse(int iterations, double a, double b) {
    int count_inside = 0;
    
    for (int i = 0; i < iterations; i++) {
        double x = (double)rand() / RAND_MAX * CANVAS_SIZE;
        double y = (double)rand() / RAND_MAX * CANVAS_SIZE;
        if (point_in_ellipse(x, y, a, b)) {
            count_inside++;
        }
    }

    return (double)count_inside / iterations * 100;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <a> <b>\n", argv[0]);
        return 1;
    }
    srand(time(NULL)); // So the seed changes every time we run the program
    double a = atof(argv[1]);
    double b = atof(argv[2]);

    if (a <= 0 || b <= 0) {
        printf("Error: Invalid ellipse parameters.\n");
        return 1;
    }

    double result = monte_carlo_ellipse(1000, a, b);
    printf("The percentage of the canvas covered by the ellipse is: %.2f%%\n", result);

    return 0;
}